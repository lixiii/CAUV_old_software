#include "pipelineWidget.h"

#include <QtGui>

#include <boost/thread.hpp>
#include <boost/ref.hpp>

#include <cmath>

#include <common/bash_cout.h>
#include <common/cauv_utils.h>
#include <debug/cauv_debug.h>

#include <utility/defer.h>

#include "pipelineWidgetNode.h"
#include "util.h"
#include "renderable.h"
#include "buildMenus.h"
#include "renderable/OverKey.h"
#include "renderable/box.h"
#include "renderable/node.h"
#include "renderable/menu.h"
#include "renderable/editText.h"
#include "renderable/arc.h"
#include "renderable/imgNode.h"


namespace pw{

// CAVEAT VIATOR: compare actual *arcs* by pointer, reverse arc compares equal
// to forward arc
bool operator==(arc_ptr_t a, arc_ptr_t b){
    return ((a->m_src.lock() == b->m_src.lock() && a->m_dst.lock() == b->m_dst.lock()) ||
            (a->m_dst.lock() == b->m_src.lock() && a->m_src.lock() == b->m_dst.lock()));
}

} // namespace pw

using namespace pw;

PipelineWidget::PipelineWidget(QWidget *parent, int argc, char** argv)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
      m_win_centre(), m_win_aspect(1), m_win_scale(10), m_scrolldelta(0),
      m_pixels_per_unit(1),
      m_last_mouse_pos(),
      m_overkey(boost::make_shared<ok::OverKey>(this)),
      m_cauv_node(),
      m_cauv_node_thread(boost::thread(spawnPGCN, this, argc, argv)),
      m_lock(), m_redraw_posted_lock(), m_redraw_posted(false){
    // TODO: more appropriate QGLFormat?

    // QueuedConnection should ensure updateGL is called in the main thread
    connect(this, SIGNAL(redrawPosted()), this, SLOT(updateGL()), Qt::QueuedConnection);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    initKeyBindings();
}

void PipelineWidget::initKeyBindings(){
    // Set-up hotkeys: TODO: load key bindings from file
    const char* dec_font = "sans";
    const int dec_font_size = 12;
    ok::action_ptr_t an_menu_act = boost::make_shared<ok::Action>(
        boost::bind(&PipelineWidget::addMenu, this,
            Defer(boost::function<menu_ptr_t()>(boost::bind(buildAddNodeMenu, this))),
            Defer<Point>(boost::bind(&PipelineWidget::lastMousePosition, this)),
            false
        ),
        ok::Action::null_f,
        "add node",
        boost::make_shared<Text>(this, "add node", dec_font, dec_font_size)
    );

    ok::action_ptr_t et_menu_act = boost::make_shared<ok::Action>(
        boost::bind(&PipelineWidget::testEditBoxMenu, this),
        ok::Action::f_t(),
        "test"
    );

    ok::action_ptr_t gm_msg_act = boost::make_shared<ok::Action>(
        boost::bind(&PipelineWidget::send, this,
            boost::make_shared<GraphRequestMessage>()
        ),
        ok::Action::null_f,
        "reload",
        boost::make_shared<Text>(this, "reload", dec_font, dec_font_size)
    );

    ok::action_ptr_t cp_node_act = boost::make_shared<ok::Action>(
        boost::bind(&PipelineWidget::duplicateNodeAtMouse, this),
        ok::Action::null_f,
        "duplicate",
        boost::make_shared<Text>(this, "duplicate", dec_font, dec_font_size)
    );

    ok::action_ptr_t rm_node_act = boost::make_shared<ok::Action>(
        boost::bind(&PipelineWidget::removeNodeAtMouse, this),
        ok::Action::null_f,
        "remove",
        boost::make_shared<Text>(this, "remove", dec_font, dec_font_size)
    );

    ok::action_ptr_t it_layout_act = boost::make_shared<ok::Action>(
        boost::bind(&PipelineWidget::iterateLayout, this),
        ok::Action::null_f,
        "auto-layout",
        boost::make_shared<Text>(this, "auto-layout", dec_font, dec_font_size)
    );

    m_overkey->registerKey(Qt::Key_Space, Qt::NoModifier, an_menu_act);
    m_overkey->registerKey(Qt::Key_A, Qt::NoModifier, an_menu_act);
    m_overkey->registerKey(Qt::Key_E, Qt::NoModifier, et_menu_act);
    m_overkey->registerKey(Qt::Key_R, Qt::NoModifier, gm_msg_act);
    m_overkey->registerKey(Qt::Key_D, Qt::NoModifier, cp_node_act);
    m_overkey->registerKey(Qt::Key_D, Qt::ControlModifier, cp_node_act);
    m_overkey->registerKey(Qt::Key_X, Qt::NoModifier, rm_node_act);
    m_overkey->registerKey(Qt::Key_L, Qt::NoModifier, it_layout_act);

    m_overkey->m_pos = -m_overkey->bbox().c();
}

QSize PipelineWidget::minimumSizeHint() const{
    return QSize(200, 200);
}

QSize PipelineWidget::sizeHint() const{
    return QSize(800, 800);
}

void PipelineWidget::remove(renderable_ptr_t p){
    if(!p)
        return;
    lock_t l(m_lock);
    // TODO: really need m_contents to be a set in which iterators remain
    // stable on erasing, so that this doesn't involve a search:
    renderable_list_t::iterator i, j;
    for(i = m_contents.begin(); i != m_contents.end(); i=j){
        j = i; j++;
        if(*i == p)
            m_contents.erase(i);
    }
    arc_ptr_t a;
    if(a = boost::dynamic_pointer_cast<Arc>(p))
        m_arcs.erase(a);

    // NB: the item may persist in m_receiving_move or m_owning_mouse until the
    // next mouse event: this is undesirable, but not something that can easily
    // be fixed, since remove() may be called during mouse events, in which case
    // removing items from the mentioned sets would invalidate iterators and
    // cause all sorts of general nastiness.
    // (see above note on really needing a set in which iterators remain stable
    // on erasing)

    postRedraw(0);
}

void PipelineWidget::remove(node_ptr_t n){
    if(!n)
        return;
    lock_t l(m_lock);
    m_nodes.erase(n->id());
    remove(renderable_ptr_t(n));
}

void PipelineWidget::remove(menu_ptr_t p){
    lock_t l(m_lock);
    if(m_menu == p)
        m_menu.reset();
    remove(renderable_ptr_t(p));
}

void PipelineWidget::add(renderable_ptr_t r){
    if(!r){
        warning() << "attempt to add NULL renderable";
        return;
    }
    lock_t l(m_lock);
    // TODO: sensible layout hint
    static Point add_position_delta = Point();
    if(add_position_delta.sxx() > 64000)
        add_position_delta = Point();
    else
        add_position_delta += Point(20, -10);
    MouseEvent last_mouse(*this);
    add(r, last_mouse.pos + add_position_delta);
}

void PipelineWidget::add(renderable_ptr_t r, Point const& at){
    lock_t l(m_lock);
    r->m_pos = at;
    m_contents.push_back(r);
    postRedraw(0);
}

void PipelineWidget::addMenu(menu_ptr_t r,  Point const& at, bool pressed){
    lock_t l(m_lock);
    debug() << "addMenu:" << r << at << pressed;
    if(m_menu)
        remove(m_menu);
    if(pressed){
        m_receiving_move.insert(r);
        m_owning_mouse.insert(r);
    }
    m_menu = r;
    add(r, at);
}

void PipelineWidget::addNode(node_ptr_t r){
    lock_t l(m_lock);
    m_nodes[r->id()] = r;
    add(r);
}

void PipelineWidget::addImgNode(imgnode_ptr_t r){
    lock_t l(m_lock);
    m_imgnodes[r->id()] = r;
    addNode(r);
}

node_ptr_t PipelineWidget::node(node_id const& n){
    lock_t l(m_lock);
    node_map_t::const_iterator i = m_nodes.find(n);
    if(i != m_nodes.end())
        return i->second;
    //warning() << "unknown node renderable:" << n;
    return node_ptr_t();
}

std::vector<node_ptr_t> PipelineWidget::nodes() const{
    lock_t l(m_lock);
    std::vector<node_ptr_t> r;
    for(node_map_t::const_iterator i = m_nodes.begin(); i != m_nodes.end(); i++)
        r.push_back(i->second);
    return r;
}

imgnode_ptr_t PipelineWidget::imgNode(node_id const& n){
    lock_t l(m_lock);
    imgnode_map_t::const_iterator i = m_imgnodes.find(n);
    if(i != m_imgnodes.end())
        return i->second;
    //warning() << "unknown img node renderable:" << n;
    return imgnode_ptr_t();
}

void PipelineWidget::addArc(node_id const& src, std::string const& output,
                            node_id const& dst, std::string const& input){
    node_ptr_t s = node(src);
    node_ptr_t d = node(dst);
    if(!s || !d)
        return;
    renderable_ptr_t s_no = s->outSocket(output);
    renderable_ptr_t d_ni = d->inSocket(input);
    if(!s_no || !d_ni)
        return;
    addArc(s_no, d_ni);
}

void PipelineWidget::addArc(renderable_ptr_t src,
                            node_id const& dst, std::string const& input){
    node_ptr_t d = node(dst);
    if(!d) return;
    renderable_ptr_t d_ni = d->inSocket(input);
    if(!d_ni) return;
    addArc(src, d_ni);
}

void PipelineWidget::addArc(node_id const& src, std::string const& output,
                            renderable_ptr_t dst){
    node_ptr_t s = node(src);
    if(!s) return;
    renderable_ptr_t s_no = s->outSocket(output);
    if(!s_no) return;
    addArc(s_no, dst);
}

arc_ptr_t PipelineWidget::addArc(renderable_wkptr_t src, renderable_wkptr_t dst){
    lock_t l(m_lock);
    arc_ptr_t a = boost::make_shared<Arc>(this, src, dst);
    for(arc_set_t::const_iterator i = m_arcs.begin(); i != m_arcs.end(); i++)
        if(*i == a){
            //warning() << "duplicate arc will be ignored";
            return *i;
        }
    m_arcs.insert(a);
    add(a, Point());
    return a;
}

void PipelineWidget::removeArc(node_id const& src, std::string const& output,
                               node_id const& dst, std::string const& input){
    node_ptr_t s = node(src);
    node_ptr_t d = node(dst);
    if(!s || !d)
        return;
    renderable_ptr_t s_no = s->outSocket(output);
    renderable_ptr_t d_ni = d->inSocket(input);
    if(!s_no || !d_ni)
        return;
    removeArc(s_no, d_ni);
}

void PipelineWidget::removeArc(renderable_ptr_t src,
                               node_id const& dst, std::string const& input){
    node_ptr_t d = node(dst);
    if(!d) return;
    renderable_ptr_t d_ni = d->inSocket(input);
    if(!d_ni) return;
    removeArc(src, d_ni);
}

void PipelineWidget::removeArc(node_id const& src, std::string const& output,
                               renderable_ptr_t dst){
    node_ptr_t s = node(src);
    if(!s) return;
    renderable_ptr_t s_no = s->outSocket(output);
    if(!s_no) return;
    removeArc(s_no, dst);
}

void PipelineWidget::removeArc(renderable_ptr_t src, renderable_ptr_t dst){
    lock_t l(m_lock);
    arc_ptr_t a = boost::make_shared<Arc>(this, src, dst);
    for(arc_set_t::const_iterator i = m_arcs.begin(); i != m_arcs.end(); i++)
        if(*i == a){
            remove(*i);
            return;
        }
    warning() << __func__ << "no such arc" << src <<  "->" << dst;
}

void PipelineWidget::setCauvNode(boost::shared_ptr<PipelineGuiCauvNode> c){
    if(m_cauv_node)
        warning() << "PipelineWidget::setCauvNode already set";
    m_cauv_node = c;
}

void PipelineWidget::send(boost::shared_ptr<Message> m){
    if(m_cauv_node)
        m_cauv_node->send(m);
    else
        error() << "PipelineWidget::send no associated cauv node";
}

node_ptr_t PipelineWidget::nodeAt(Point const& p) const{
    foreach(node_map_t::value_type const& v, m_nodes){
        if(v.second->bbox().contains(p - v.second->m_pos))
            return v.second;
    }
    return node_ptr_t();
}

Point PipelineWidget::referUp(Point const& p) const{
    // nowhere up to refer to: just return the same point
    return p;
}

void PipelineWidget::postRedraw(float delay){
    lock_t l2(m_redraw_posted_lock);
    // no need to lock m_lock, that's done in the slot
    if(delay != 0.0f){
        // allow immediate redraws to still get through...
        //m_redraw_posted = true;
        if(delay != 0.0f){
            QTimer::singleShot(delay * 1000, this, SIGNAL(redrawPosted()));
            debug(3) << "PipelineWidget::postRedraw re-draw signal with delay=" << delay;
        }else{
            debug(3) << "PipelineWidget::postRedraw emitting re-draw signal";
            emit redrawPosted();
        }
    }else{
        if(!m_redraw_posted){
            m_redraw_posted = true;
            emit redrawPosted();
        }else{
            debug(3) << BashColour::Red << "PipelineWidget::postRedraw NOT emitting re-draw signal";
        }
    }
}

void PipelineWidget::postMenu(menu_ptr_t m, Point const& p, bool r) {
    addMenu(m, p, r);
}

void PipelineWidget::removeMenu(menu_ptr_t r){
    remove(r);
}


void PipelineWidget::initializeGL(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glCullFace(GL_BACK);

    glClearColor(0, 0, 0, 1.0);
    glClearDepth(100.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PipelineWidget::paintGL(){
    lock_t l1(m_lock);

    debug(3) << "PipelineWidget::paintGL";
    updateProjection();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    // scale for world size 'out of screen' is positive z
    glScalef(m_pixels_per_unit / m_world_size,
             m_pixels_per_unit / m_world_size, 1.0f);
    //glTranslatef(m_pixels_per_unit / 2, m_pixels_per_unit / 2, 0);

    // the grid is somewhat special, since what is drawn depends on the
    // projection
    drawGrid();
    glClear(GL_DEPTH_BUFFER_BIT);

    #if 0
    // debug stuff:
    glBegin(GL_LINES);
    glColor4f(1.0, 0.0, 0.0, 0.5);
    glVertex2f(-10.0f, 0.0f);
    glVertex2f(10.0f, 0.0f);

    glColor4f(0.0, 1.0, 0.0, 0.5);
    glVertex2f(0.0f, -10.0f);
    glVertex2f(0.0f, 10.0f);

    glColor4f(1.0, 0.0, 0.0, 0.5);
    glVertex2f(-20.0f, -17.5f);
    glVertex2f(-15.0f, -17.5f);

    glColor4f(0.0, 1.0, 0.0, 0.5);
    glVertex2f(-17.5f, -20.0f);
    glVertex2f(-17.5f, -15.0f);
    glEnd();
    #endif

    foreach(renderable_ptr_t r, m_contents){
        glPushMatrix();
        glTranslatef(r->m_pos);
        r->draw(drawtype_e::no_flags);
        glPopMatrix();
    }

    // draw overlays:
    glClear(GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glScalef(1.0f / m_world_size, 1.0f / m_world_size, 1.0f);
    // erm... undo the projection transformation: TODO: separate updateProjection for overlays so this isn't necessary
    glTranslatef(-m_win_centre*m_pixels_per_unit);
    glTranslatef(m_overkey->m_pos);
    m_overkey->draw(drawtype_e::no_flags);

    glPrintErr();

    l1.unlock();
    lock_t l2(m_redraw_posted_lock);
    m_redraw_posted = false;
}

void PipelineWidget::resizeGL(int width, int height){
    m_win_aspect = sqrt(double(width) / height);
    m_win_scale = sqrt(width*height);
    debug(2) << __func__
            << "width=" << width << "height=" << height
            << "aspect=" << m_win_aspect << "scale=" << m_win_scale;

    glViewport(0, 0, width, height);

    updateProjection();
}

void PipelineWidget::mousePressEvent(QMouseEvent *event){
    lock_t l(m_lock);
	GLuint hits = 0;
    GLuint n = 0;
    GLuint pick_buffer[100] = {0};
    GLuint *p;
    GLuint *item;
    renderable_list_t::iterator i;

    typedef std::map<GLuint, renderable_ptr_t> name_map_t;
    name_map_t name_map;

	glSelectBuffer(sizeof(pick_buffer)/sizeof(GLuint), pick_buffer);
	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(GLuint(-1));

    projectionForPicking(event->x(), event->y());
    glLoadIdentity();
    glScalef(m_pixels_per_unit / m_world_size,
             m_pixels_per_unit / m_world_size, 1.0f);
    //glTranslatef(m_pixels_per_unit / 2, m_pixels_per_unit / 2, 0);

    for(i = m_contents.begin(); i != m_contents.end(); i++, n++){
        if((*i)->acceptsMouseEvents()){
			glLoadName(n);
            name_map[n] = *i;
            glPushMatrix();
            glTranslatef((*i)->m_pos);
            (*i)->draw(drawtype_e::picking);
            glPopMatrix();
        }
    }
    glPopName();
    glFlush();
    hits = glRenderMode(GL_RENDER);
    debug() << "rendered" << n << "items for pick," << hits << "hit";

    bool need_redraw = false;

    if(m_menu){
        remove(m_menu);
        need_redraw = true;
    }

    p = pick_buffer;
    for(unsigned i = 0; i < hits; i++, p += (*p) + 3){
		item = p+3;
		for(unsigned j = 0; j < *p; j++, item++){
			name_map_t::const_iterator k = name_map.find(*item);
            if(k == name_map.end()){
                error() << "gl name" << *item << "does not correspond to renderable";
            }else{
                debug(2) << "sending mouse press event to" << k->second;
                k->second->mousePressEvent(MouseEvent(event, k->second, *this));
                m_owning_mouse.insert(k->second);
            }
		}
	}

    if(event->buttons() & Qt::RightButton){
        MouseEvent proxy(event, *this);
        addMenu(buildAddNodeMenu(this), proxy.pos);
        need_redraw = true;
    }

    m_last_mouse_pos = event->pos();

    if(need_redraw)
        postRedraw(0);
}

void PipelineWidget::keyPressEvent(QKeyEvent* event){
    lock_t l(m_lock);
    if((m_menu && !m_menu->keyPressEvent(event)) || !m_menu)
        m_overkey->keyPressEvent(event);
        //if(!m_overkey->keyPressEvent(event))
        //    QWidget::keyPressEvent(event);
}

void PipelineWidget::keyReleaseEvent(QKeyEvent* event){
    lock_t l(m_lock);
    if((m_menu && !m_menu->keyReleaseEvent(event)) || !m_menu)
        m_overkey->keyReleaseEvent(event);
        //if(!m_overkey->keyReleaseEvent(event))
        //    QWidget::keyReleaseEvent(event);
}

void PipelineWidget::wheelEvent(QWheelEvent *event){
    lock_t l(m_lock);
    debug(2) << __func__ << event->delta() << std::hex << event->buttons();
    if(!event->buttons()){
        m_scrolldelta += event->delta();
        double scalef = pow(1.2, double(m_scrolldelta / 240));
        m_pixels_per_unit = clamp(0.04, scalef, 4);
        postRedraw(0);
    }
}

void PipelineWidget::mouseReleaseEvent(QMouseEvent *event){
    lock_t l(m_lock);
    renderable_set_t::iterator i;
    for(i = m_owning_mouse.begin(); i != m_owning_mouse.end(); i++){
       debug(2) << "sending mouse release event to" << *i;
       (*i)->mouseReleaseEvent(MouseEvent(event, *i, *this));
    }
    if(!event->buttons()){
        m_owning_mouse.clear();
    }
}

void PipelineWidget::mouseMoveEvent(QMouseEvent *event){
    lock_t l(m_lock);
    renderable_set_t::iterator i;
    renderable_list_t::iterator j;
    if(!m_owning_mouse.size()){
        int win_dx = event->x() - m_last_mouse_pos.x();
        int win_dy = event->y() - m_last_mouse_pos.y();
        if(event->buttons() & Qt::LeftButton){
            Point d(win_dx / m_pixels_per_unit,
                    -win_dy / m_pixels_per_unit); // NB -
            m_win_centre += d;
            postRedraw(0);
        }
        // else zoom, (TODO)
    }else{
        for(i = m_owning_mouse.begin(); i != m_owning_mouse.end(); i++){
            debug(2) << "sending mouse move event to" << *i;
            (*i)->mouseMoveEvent(MouseEvent(event, *i, *this));
        }
    }
    renderable_set_t now_receiving_move = m_owning_mouse;
    // in all cases send the event to things that always track mouse movement:
    for(j = m_contents.begin(); j != m_contents.end(); j++)
        if((*j)->tracksMouse() && !m_owning_mouse.count(*j)){
            MouseEvent m = MouseEvent(event, *j, *this);
            if((*j)->bbox().contains(m.pos)){
                (*j)->mouseMoveEvent(m);
                now_receiving_move.insert(*j);
            }
        }
    // and send mouseGoneEvents to things that the mouse has left
    for(i = m_receiving_move.begin(); i != m_receiving_move.end(); i++)
        if(!now_receiving_move.count(*i))
            (*i)->mouseGoneEvent();
    m_receiving_move = now_receiving_move;

    m_last_mouse_pos = event->pos();
}

void PipelineWidget::updateProjection(){
    double w = (m_win_scale * m_win_aspect) / m_world_size;
    double h = (m_win_scale / m_win_aspect) / m_world_size;
    debug(2) << __func__
            << "w=" << w << "h=" << h
            << "aspect=" << m_win_aspect << "scale=" << m_win_scale
            << "res=" << m_pixels_per_unit
            << "wc=" << m_win_centre;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glTranslatef(0.375 * w / width(), 0.375 * h / height(), 0);
    glOrtho(-w/2, w/2, -h/2, h/2, -100, 100);
    glTranslatef(m_win_centre * m_pixels_per_unit / m_world_size);
    glMatrixMode(GL_MODELVIEW);
}

void PipelineWidget::projectionForPicking(int mouse_win_x, int mouse_win_y){
    double w = (m_win_scale * m_win_aspect) / m_world_size;
    double h = (m_win_scale / m_win_aspect) / m_world_size;
    debug(2) << __func__
            << "w=" << w << "h=" << h
            << "aspect=" << m_win_aspect << "scale=" << m_win_scale
            << "res=" << m_pixels_per_unit
            << "wc=" << m_win_centre;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix((GLdouble)mouse_win_x, (GLdouble)(viewport[3]-mouse_win_y),
                  1, 1, viewport);

    //glTranslatef(0.375 * w / width(), 0.375 * h / height(), 0);
    glOrtho(-w/2, w/2, -h/2, h/2, -100, 100);
    glTranslatef(m_win_centre * m_pixels_per_unit / m_world_size);
    glMatrixMode(GL_MODELVIEW);
}

void PipelineWidget::drawGrid(){
    // only draw grid that will be visible:
    const double grid_major_spacing = 250;
    const double grid_minor_spacing = 25;

    // projected window coordinates:
    const double divisor = 2 * m_pixels_per_unit;
    const double edge = m_world_size * m_win_scale / 2;
    const double min_x = max(-m_win_centre.x - width()  / divisor, -edge);
    const double min_y = max(-m_win_centre.y - height() / divisor, -edge);
    const double max_x = min(-m_win_centre.x + width()  / divisor, edge);
    const double max_y = min(-m_win_centre.y + height() / divisor, edge);

    const int min_grid_minor_x = roundZ(min_x / grid_minor_spacing);
    const int min_grid_minor_y = roundZ(min_y / grid_minor_spacing);
    const int max_grid_minor_x = roundZ(max_x / grid_minor_spacing);
    const int max_grid_minor_y = roundZ(max_y / grid_minor_spacing);

    const int min_grid_major_x = roundZ(min_x / grid_major_spacing);
    const int min_grid_major_y = roundZ(min_y / grid_major_spacing);
    const int max_grid_major_x = roundZ(max_x / grid_major_spacing);
    const int max_grid_major_y = roundZ(max_y / grid_major_spacing);

    debug(3) << "min_x=" << min_x << "max_x=" << max_x
            << "min_y=" << min_y << "max_y=" << max_y << "\n\t"
            << "min_grid_minor_x=" << min_grid_minor_x
            << "max_grid_minor_x=" << max_grid_minor_x << "\n\t"
            << "min_grid_minor_y=" << min_grid_minor_y
            << "max_grid_minor_y=" << max_grid_minor_y << "\n\t"
            << "min_grid_major_x=" << min_grid_major_x
            << "max_grid_major_x=" << max_grid_major_x << "\n\t"
            << "min_grid_major_y=" << min_grid_major_y
            << "max_grid_major_y=" << max_grid_major_y;

    glLineWidth(round(clamp(0, m_pixels_per_unit, 5)));
    const float sqrp = std::sqrt(m_pixels_per_unit);
    const float alpha_div = sqrp < 1? 1/sqrp : 1.0f;
    glColor(Colour(0.2, 0.3/alpha_div));
    glBegin(GL_LINES);
    for(int i = min_grid_minor_y; i <= max_grid_minor_y; i++){
        glVertex3f(min_x, i*grid_minor_spacing, -0.2);
        glVertex3f(max_x, i*grid_minor_spacing, -0.2);
    }

    for(int i = min_grid_minor_x; i <= max_grid_minor_x; i++){
        glVertex3f(i*grid_minor_spacing, min_y, -0.2);
        glVertex3f(i*grid_minor_spacing, max_y, -0.2);
    }

    glColor(Colour(0.2, 0.6/alpha_div));
    for(int i = min_grid_major_y; i <= max_grid_major_y; i++){
        glVertex3f(min_x, i*grid_major_spacing, -0.1);
        glVertex3f(max_x, i*grid_major_spacing, -0.1);
    }

    for(int i = min_grid_major_x; i <= max_grid_major_x; i++){
        glVertex3f(i*grid_major_spacing, min_y, -0.1);
        glVertex3f(i*grid_major_spacing, max_y, -0.1);
    }
    glEnd();
}


PipelineWidget::node_set_t PipelineWidget::parents(node_id n) const{
    node_set_t r;
    foreach(arc_ptr_t a, m_arcs)
        if(!a->m_hanging && a->to().node == n && a->from().node)
            r.insert(a->from().node);
    return r;
}

PipelineWidget::node_set_t PipelineWidget::children(node_id n) const{
    node_set_t r;
    foreach(arc_ptr_t a, m_arcs)
        if(!a->m_hanging && a->from().node == n && a->to().node)
            r.insert(a->to().node);
    return r;
}

// TODO: move this somewhere appropriate... probably a member function
void tdf(int, std::string const& s){
    debug() << "Edit done:" << s;
}

Point PipelineWidget::lastMousePosition() const{
    MouseEvent proxy(*this);
    return proxy.pos;
}

void PipelineWidget::duplicateNodeAtMouse(){
    debug() << __func__;
    node_ptr_t current_node = nodeAt(lastMousePosition());
    if(current_node)
        send(boost::make_shared<AddNodeMessage>(
            current_node->type(),
            std::vector<NodeInputArc>(),
            std::vector<NodeOutputArc>()
        ));
}

void PipelineWidget::removeNodeAtMouse(){
    debug() << __func__;
    node_ptr_t current_node = nodeAt(lastMousePosition());
    if(current_node)
        send(boost::make_shared<RemoveNodeMessage>(
            current_node->id()
        ));
}

void PipelineWidget::testEditBoxMenu(){
    debug() << __func__;
    addMenu(boost::make_shared< EditText<int> >(
        this, std::string("edit here"), BBox(0, -6, 80, 10), tdf, 0
    ), lastMousePosition());
    postRedraw(0);
}


static float mass(node_ptr_t){
    return 1.0f;
}

void PipelineWidget::iterateLayout(){
    // - overlapping nodes repel each other, and all nodes repel each other
    // - arcs attract in a straight line with squared growth (but small)
    //
    // ... yes, this function is O(n**2) in nodes, and O(n) in arcs, it should
    // probably be better than that if it is going to handle large layouts
    // smoothly
    
    const float scale        =  0.01f;
    const float area_enlarge =  1.10f;
    const float node_area_1  =  0.50f * scale;
    const float node_dist_0  =  1.00f * scale;
    const float node_dist_1  = -1.00f * scale;
    const float node_dist_2  =  0.00f * scale;
    const float arc_length_0 =  1.00f * scale;
    const float arc_length_1 =  0.00f * scale;
    const float arc_length_2 = -0.01f * scale;

    typedef V2D<float> vec_t;

    node_map_t::iterator n1, n2;
    node_ptr_t np1, np2;
    renderable_ptr_t ah1, ah2;
    vec_t displ, force;
    BBox overlap;
    
    // arc forces:
    foreach(arc_ptr_t a, m_arcs)
        if(!a->m_hanging && a->to().node && a->from().node &&
           (n1 = m_nodes.find(a->from().node)) != m_nodes.end() &&
           (n2 = m_nodes.find(a->to().node)) != m_nodes.end()){
            ah1 = a->fromOutput();
            ah2 = a->toInput();
            np1 = n1->second;
            np2 = n2->second;
            displ = vec_t(np2->m_pos + ah2->m_pos - np1->m_pos - ah1->m_pos);
            // this force never repels
            force = displ.unit() * min<float>(0.0f,
                arc_length_0 +
                displ.len() * arc_length_1 +
                displ.sxx() * arc_length_2
            );
            debug(5) << "arc" << np1 << "<->" << np2
                     << "displ=" << displ << "force=" << force;
            np1->m_pos -= Point(force / mass(np1));
            np2->m_pos += Point(force / mass(np2));
        }
    
    // overlaps & distances
    for(n1 = m_nodes.begin(); n1 != m_nodes.end(); n1++){
        np1 = n1->second;
        n2 = n1;
        n2++;
        for(; n2 != m_nodes.end(); n2++){
            np2 = n2->second;
            overlap = (np1->bbox() * area_enlarge + np1->m_pos) &
                      (np2->bbox() * area_enlarge + np2->m_pos);
            displ = vec_t(np2->m_pos - np1->m_pos);
            if(displ.sxx() == 0)
                displ = vec_t(0.01, 0.01);
            // this force never attracts:
            force = displ.unit() * max<float>(0.0f,
                overlap.area() * node_area_1 +
                node_dist_0 + 
                displ.len() * node_dist_1 +
                displ.sxx() * node_dist_2
            ); 
            debug(5) << "area/dist" << np1 << "<->" << np2
                     << "displ=" << displ
                     << "(len=" << displ.len() << "sxx=" << displ.sxx() << ")"
                     << "area=" << overlap.area()
                     << "force=" << force;
            np1->m_pos -= Point(force / mass(np1));
            np2->m_pos += Point(force / mass(np2));
        }
    }

    postRedraw(0);
}
