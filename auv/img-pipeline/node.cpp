#include "node.h"

#include "imageProcessor.h"
#include "pipelineTypes.h"
#include "nodeFactory.h"

using namespace cauv;
using namespace cauv::imgproc;

const char* Node::Image_In_Name = "image in";
const char* Node::Image_Out_Name = "image out (not copied)";
const char* Node::Image_Out_Copied_Name = "image out";


Node::Node(Scheduler& sched, ImageProcessor& pl, NodeType::e type)
    : m_priority(priority_slow),
      m_speed(slow),

      m_node_type(type),
      m_id(_newID()),

      m_parent_links(),
      m_parent_links_lock(),

      m_child_links(),
      m_child_links_lock(),

      m_outputs(),
      m_outputs_lock(),

      m_parameters(),
      m_parameter_tips(),
      m_parameters_lock(),

      m_checking_sched_lock(),
      m_exec_queued(false),
      m_exec_queued_lock(),
      m_new_inputs(),
      m_new_inputs_lock(),
      m_valid_inputs(),
      m_valid_inputs_lock(),
      m_new_paramvalues(),
      m_new_paramvalues_lock(),
      m_output_demanded(false),
      m_output_demanded_on(),
      m_output_demanded_lock(),
      m_allow_queue(true),
      m_allow_queue_lock(),

      m_sched(sched),
      m_pl(pl){
}

Node::~Node(){
    debug() << "~Node" << *this;
}

void Node::stop(){
    debug(-3) << BashColour::Purple << "stop()" << *this << "check...";
    // check that the node is not executing: if it is then there should still
    // be a shared pointer hanging around, and this node should not be being
    // destroyed, so complain loudly!
    int err = 0;
    if(!m_parent_links_lock.try_lock())    err |= 1;
    if(!m_child_links_lock.try_lock())     err |= 1 << 1;
    if(!m_outputs_lock.try_lock())         err |= 1 << 2;
    if(!m_parameters_lock.try_lock())      err |= 1 << 3;
    if(!m_checking_sched_lock.try_lock())  err |= 1 << 4;
    if(!m_exec_queued_lock.try_lock())     err |= 1 << 5;
    if(!m_new_inputs_lock.try_lock())      err |= 1 << 6;
    if(!m_valid_inputs_lock.try_lock())    err |= 1 << 7;
    if(!m_new_paramvalues_lock.try_lock()) err |= 1 << 8;
    if(!m_output_demanded_lock.try_lock()) err |= 1 << 9;
    if(!m_allow_queue_lock.try_lock())     err |= 1 << 10;
    
    if(err)
        error() << this << "SUPER BADNESS: destruct with locked mutexes:" << err;

    debug(-3) << BashColour::Purple << "stop()" << *this << ", done";
}

NodeType::e const& Node::type() const{
    return m_node_type;
}

node_id const& Node::id() const{
    return m_id;
}

/* overload for the common case where we're connecting a node with one
 * output to a node with one input
 */
void Node::setInput(node_ptr_t n){
    unique_lock_t l(m_parent_links_lock);
    if(m_parent_links.size() == 1){
        const output_id_set_t parent_outputs = n->outputs();
        if(parent_outputs.size() == 1){
            in_link_map_t::value_type pl = *m_parent_links.begin();
            l.unlock();
            setInput(pl.first, pl.second.node, pl.second.id);
        }else{
            error() << "Parent has " << parent_outputs.size() << " outputs!";
            throw link_error("setInput: specific parent output must be specified");
        }
    }else if(m_parent_links.size() > 1){
        throw link_error("setInput: specific input must be specified");
    }else{
        throw link_error("setInput: This node accepts no inputs");
    }
}

void Node::setInput(input_id const& i_id, node_ptr_t n, output_id const& o_id){
    unique_lock_t l(m_parent_links_lock);
    const in_link_map_t::iterator i = m_parent_links.find(i_id);
    if(i == m_parent_links.end()){
        throw id_error("setInput: Invalid input id" + toStr(i_id));
    }else if(n->id() == id()){
        throw link_error("can't link nodes to themselves: blame shared mutexes being non-recursive");
    }else if(m_parameters.count(i->first) != n->paramOutputs().count(o_id)){
        throw link_error("setInput: Parameter <==> Image mismatch");
    }else{
        param_id param = i->first;
        if(i->second.node){
            throw link_error("old arc must be removed first");
        }
        debug(-3) << BashColour::Green << "adding parent link on" << i_id << "->" << *n << o_id;
        i->second = input_link_t(n, o_id);
        l.unlock();
        if(m_parameters.count(param)){
            warning() << "assuming output parameter is available, this needs fixing";
            setNewParamValue(param);
        }else{
            if(n->getOutputImage(o_id, true)){
                debug(4) << "node" << *this << "input set, output available from" << *n;
                setNewInput(param);
            }else{
                debug(4) << "node" << *this << "input set, demand new output on" << *n;
                n->setNewOutputDemanded(o_id);
            }
        }
        n->setNewOutputDemanded(o_id);
    }
}

void Node::clearInput(input_id const& i_id){
    unique_lock_t l(m_parent_links_lock);
    const in_link_map_t::iterator i = m_parent_links.find(i_id);
    if(i == m_parent_links.end()){
        throw id_error("clearInput: Invalid input id" + toStr(i_id));
    }else{
        debug(-3) << BashColour::Purple << "removing parent link on " << i_id;
        i->second = input_link_t();
    }
}

void Node::clearInputs(node_ptr_t parent){
    unique_lock_t l(m_parent_links_lock);
    in_link_map_t::iterator i;
    debug(-3) << BashColour::Purple << "removing parent links to" << *parent;
    foreach(in_link_map_t::value_type& v, m_parent_links){
        input_link_t& link = v.second;
        if(link.node == parent)
            link = input_link_t();
    }
}

void Node::clearInputs(){
    unique_lock_t l(m_parent_links_lock);
    in_link_map_t::iterator i;
    foreach(in_link_map_t::value_type& v, m_parent_links){
        input_link_t& link = v.second;
        link = input_link_t();
    }
}

Node::input_id_set_t Node::inputs() const{
    input_id_set_t r;
    shared_lock_t n(m_parent_links_lock);
    in_link_map_t::const_iterator i;
    foreach(in_link_map_t::value_type const& v, m_parent_links){
        const input_id& id = v.first;
        if(!m_parameters.count(id)) // parameters don't count!
            r.insert(id);
    }
    return r;
}

Node::msg_node_input_map_t Node::inputLinks() const{
    shared_lock_t l(m_parent_links_lock);
    msg_node_input_map_t r;
    foreach(in_link_map_t::value_type const& v, m_parent_links){
        const input_id& id = v.first;
        const input_link_t& link = v.second;
        // parameters _do_ count
        NodeOutput t;
        t.node = m_pl.lookup(link.node);
        t.output = link.id;
        // link.first is guaranteed to not be this since nodes self links
        // are not allowed
        if(link.node && link.node->paramOutputs().count(link.id))
            t.type = OutputType::Parameter;
        else
            t.type = OutputType::Image;
        r[id] = t;
    }
    return r;
}

std::set<node_ptr_t> Node::parents() const{
    unique_lock_t l(m_parent_links_lock);
    std::set<node_ptr_t> r;
    foreach(in_link_map_t::value_type const& i, m_parent_links)
        r.insert(i.second.node); // parameters _do_ count
    return r;
}


/*  overload for the common case where we're connecting a node with one
 *  output to a node with one input
 */
void Node::setOutput(node_ptr_t n){
    unique_lock_t l(m_child_links_lock);
    if(m_child_links.size() == 1){
        const input_id_set_t child_inputs = n->inputs();
        if(child_inputs.size() != 1){
            throw link_error("setOutput: specific child input must be specified");
        }
        output_id output = m_child_links.begin()->first;
        input_id input = *child_inputs.begin();
        l.unlock();
        setOutput(output, n, input);
    }else if(m_child_links.size() > 1){
        throw link_error("setOutput: specific output must be specified");
    }else{
        throw link_error("setOutput: this node has no outputs");
    }
}

void Node::setOutput(output_id const& o_id, node_ptr_t n, input_id const& i_id){
    // check this first, because paramOutputs locks m_child_links_lock
    if(paramOutputs().count(o_id) != n->parameters().count(i_id)){
        throw link_error("setInput: Parameter <==> Image mismatch");
    }
    unique_lock_t l(m_child_links_lock);
    const out_link_map_t::iterator i = m_child_links.find(o_id);
    if(i == m_child_links.end()){
        throw id_error("setOutput: Invalid output id" + toStr(o_id));
    }else if(n->id() == id()){
        throw link_error("can't link nodes to themselves: blame shared mutexes being non-recursive");
    }else{
        // An output can be connected to more than one input, so
        // m_child_links[output_id] is a list of output_link_t
        debug(-3) << BashColour::Green << "adding output link to child: " << *n << i_id;
        i->second.push_back(output_link_t(n, i_id));
    }
}

void Node::clearOutput(output_id const& o_id, node_ptr_t n, input_id const& i_id){
    unique_lock_t l(m_child_links_lock);
    const out_link_map_t::iterator i = m_child_links.find(o_id);
    if(i == m_child_links.end()){
        throw id_error("clearOutput: Invalid output id" + toStr(o_id));
    }else{
        // An output can be connected to more than one input, so
        // m_child_links[output_id] is a list of output_link_t
        output_link_list_t::iterator j = std::find(i->second.begin(), i->second.end(), output_link_t(n, i_id));
        if(j == i->second.end()){
            throw id_error("clearOutput: Invalid node & input id: "
                           + toStr(m_pl.lookup(n)) + ", "
                           + toStr(i_id));
        }else{
            debug(-3) << BashColour::Purple << "removing output link to child:" << j->node << j->id;
            i->second.erase(j);
        }
    }
}

template<typename T>
struct FirstIs{
    FirstIs(typename T::node_t const& node): m_node(node){ }
    bool operator()(T const& v){ return v.node == m_node; }
    typename T::node_t m_node;
};
void Node::clearOutputs(node_ptr_t child){
    unique_lock_t l(m_child_links_lock);
    debug(-3) << BashColour::Purple << "removing output links to child:" << *child;
    foreach(out_link_map_t::value_type& i, m_child_links)
        i.second.remove_if(FirstIs<output_link_t>(child));
}

void Node::clearOutputs(){
    unique_lock_t l(m_child_links_lock);
    foreach(out_link_map_t::value_type& i, m_child_links){
        debug(-3) << BashColour::Purple << "removing output link to children:"
                 << i.first << "->" << i.second;
        i.second.clear();
    }
}

Node::output_id_set_t Node::outputs(int type_index) const{
    output_id_set_t r;
    shared_lock_t n(m_child_links_lock);
    foreach(out_link_map_t::value_type const& i, m_child_links){
        const out_map_t::const_iterator j = m_outputs.find(i.first);
        assert(j != m_outputs.end());
        if(j->second.which() == type_index)
            r.insert(i.first);
    }
    return r;
}

Node::output_id_set_t Node::outputs() const{
    return outputs(0);
}

Node::output_id_set_t Node::paramOutputs() const{
    return outputs(1);
}

Node::msg_node_output_map_t Node::outputLinks() const{
    shared_lock_t l(m_child_links_lock);
    msg_node_output_map_t r;
    foreach(out_link_map_t::value_type const& i, m_child_links){
        msg_node_in_list_t input_list;
        foreach(output_link_list_t::value_type const& j, i.second){
            NodeInput t;
            t.node = m_pl.lookup(j.node);
            t.input = j.id;
            input_list.push_back(t);
        }
        r[i.first] = input_list;
    }
    return r;
}

std::set<node_ptr_t> Node::children() const{
    shared_lock_t l(m_child_links_lock);
    std::set<node_ptr_t> r;
    foreach(out_link_map_t::value_type const& i, m_child_links)
        foreach(output_link_list_t::value_type const& j, i.second)
            r.insert(j.node);
    return r;
}

int Node::numChildren() const{
    int r = 0;
    shared_lock_t l(m_child_links_lock);
    foreach(out_link_map_t::value_type const& i, m_child_links)
        r += i.second.size();
    return r;
}

// enum status mangling:
static NodeStatus::e& operator|=(NodeStatus::e& l, NodeStatus::e const& r){
    return l = NodeStatus::e(unsigned(l) | unsigned(r));
}
static NodeStatus::e operator|(NodeStatus::e const& l, NodeStatus::e const& r){
    return NodeStatus::e(unsigned(l) | unsigned(r));
}

//static NodeIOStatus::e& operator|=(NodeIOStatus::e& l, NodeIOStatus::e const& r){
//    return l = NodeIOStatus::e(unsigned(l) | unsigned(r));
//}
static NodeIOStatus::e operator|(NodeIOStatus::e const& l, NodeIOStatus::e const& r){
    return NodeIOStatus::e(unsigned(l) | unsigned(r));
}

// there must be a nicer way to do this...
#define CallOnDestruct(Type, member) \
struct _COD{_COD(Type& m):m(m){}~_COD(){m.member();}Type& m;}
void Node::exec(){
    CallOnDestruct(Node, clearExecQueued) cod(*this);
    // take copies of image_ptr s from parents before _demandNewParentInput()
    in_image_map_t inputs;
    out_map_t outputs;

    shared_lock_t pl(m_parent_links_lock);
    shared_lock_t rl(m_parameters_lock);
    
    bool bad_input = false;
    foreach(in_link_map_t::value_type const& v, m_parent_links){
        if(!m_parameters.count(v.first)){
            if(!v.second.node){
                warning() << "exec: no parent on:" << v.first;
                clearValidInput(v.first);
                bad_input = true;
            }else{
                inputs[v.first] = v.second.node->getOutputImage(v.second.id);
                if(!inputs[v.first]){
                    warning() << "exec: no output from: " << v.second << "to" << v.first;
                    clearValidInput(v.first);
                    bad_input = true;
                }
            }
        }
    }
    if(bad_input)
        return;

    rl.unlock();
    
    // Record that we've used all of our inputs with the current parameters
    clearNewInput();
    clearNewParamValues();

    pl.unlock();

    debug(4) << "exec: id=" << m_id << "type=" << m_node_type
             << "speed=" << m_speed << ", " << inputs.size() << "inputs";
    
    NodeStatus::e status = NodeStatus::e(0);
    if(allowQueue()) status |= NodeStatus::AllowQueue;
    _statusMessage(boost::make_shared<StatusMessage>(m_id, status | NodeStatus::Executing));
    try{
        if(this->m_speed < medium){
            // if this is a fast node: request new image from parents before executing
            _demandNewParentInput();
            outputs = this->doWork(inputs);
        }else{
            // if this is a slow node, request new images from parents after executing
            outputs = this->doWork(inputs);
            _demandNewParentInput();
        }
    }catch(std::exception& e){
        error() << "Error executing node: " << *this << "\n\t" << e.what();
    }
    _statusMessage(boost::make_shared<StatusMessage>(m_id, status));

    unique_lock_t ol(m_outputs_lock);
    shared_lock_t cl(m_child_links_lock);
    foreach(out_map_t::value_type& v, outputs){
        if(!m_outputs.count(v.first)){
            warning() << "exec() produced output at an unknown id:" << v.first
                      << "(ignored)";
        }else{            
            if(m_outputs[v.first].which() == v.second.which()){
                clearNewOutputDemanded(v.first);
                m_outputs[v.first] = v.second;
                out_link_map_t::iterator kids = m_child_links.find(v.first);
                if(kids != m_child_links.end()){
                    debug(5) << "Prompting" << kids->second.size() << "children of new output:";
                    // for each node connected to the output
                    foreach(output_link_t& link, kids->second){
                        // link is a std::pair<node_ptr, input_id>
                        debug(5) << "prompting new input to child on:" << v.first;
                        // notify the node that it has new input
                        if(link.node)
                            link.node->setNewInput(link.id);
                    }
                }else{
                    debug(5) << "no children to prompt";
                }
            }else{
                warning() << "exec() produced output of the wrong type for id:"
                          << v.first << "(ignored)";
            }
        }
    }
    // warn if no outputs were filled
    if(0 == outputs.size() && m_outputs.size())
        warning() << *this << "exec() produced no output when some was expected";
    // warn about any outputs that weren't filled
    //foreach(out_link_map_t::value_type& v, m_child_links)
    //    if(!outputs.count(v.first))
    //        warning() << "exec() did not fill output:" << v.first << "\n\t"
    //                  << v.second.size() << "children will not be prompted";
    cl.unlock();
    ol.unlock();
}

/* Get the actual image data associated with an output
 */
Node::image_ptr_t Node::getOutputImage(output_id const& o_id,
                                       bool suppress_null_warning) const throw(id_error){
    shared_lock_t l(m_outputs_lock);
    const out_map_t::const_iterator i = m_outputs.find(o_id);
    image_ptr_t r;
    if(i != m_outputs.end()){
        try{
            r = boost::get<image_ptr_t>(i->second);
        }catch(boost::bad_get&){
            throw id_error("requested output is not an image_ptr_t" + toStr(o_id));
        }
    }else{
        throw id_error("no such output" + toStr(o_id));
    } 
    if(!r && !suppress_null_warning)
        warning() << m_id << "returning NULL image for" << o_id;
    return r;
}

NodeParamValue Node::getOutputParam(output_id const& o_id) const throw(id_error){
    shared_lock_t l(m_outputs_lock);
    const out_map_t::const_iterator i = m_outputs.find(o_id);
    NodeParamValue r;
    if(i != m_outputs.end()){
        try{
            r = boost::get<NodeParamValue>(i->second);
        }catch(boost::bad_get&){
            throw id_error("requested output is not a NodeParamValue" + toStr(o_id));
        }
    }else{
        throw id_error("no such output" + toStr(o_id));
    }
    return r;
}

/* return all parameter values
 */
std::map<param_id, NodeParamValue> Node::parameters() const{
    shared_lock_t l(m_parameters_lock);
    return m_parameters;
}

/* set a parameter based on a message
 */
void Node::setParam(boost::shared_ptr<const SetNodeParameterMessage>  m){
    debug(1) << "Node::setParam" << m;
    setParam(m->paramId(), m->value());
}

void Node::registerInputID(input_id const& i){
    unique_lock_t l(m_new_inputs_lock);
    unique_lock_t m(m_valid_inputs_lock);
    unique_lock_t n(m_parent_links_lock);

    m_new_inputs[i] = false;
    m_valid_inputs[i] = false;
    m_parent_links[i] = input_link_t();

    _statusMessage(boost::make_shared<InputStatusMessage>(m_id, i, NodeIOStatus::e(0)));
}

/* Check to see if all inputs are new and output is demanded; if so,
 * add this node to the scheduler queue
 */
void Node::checkAddSched(SchedMode m){
    unique_lock_t l(m_checking_sched_lock);
    if(!allowQueue()){
        debug(4) << __func__ << "Cannot enqueue node" << *this << ", allowQueue false";
        return;
    }
    if(execQueued()){
        debug(4) << __func__ << "Cannot enqueue node" << *this << ", exec queued already";
        return;
    }

    const bool out_demanded = newOutputDemanded();
    const bool new_pvs = newParamValues();
    if(m != Always && out_demanded == false && new_pvs == false){
        debug(4) << __func__ << "Cannot enqueue node" << *this << ", no demand:"
                 << "new params=" << new_pvs << "output demand=" << out_demanded;
        return;
    }

    if(!validInputAll()){
        debug(4) << __func__ << "Cannot enqueue node" << *this << ", input is invalid";
        return;
    }
    // a new paramvalue is not sufficient to trigger repeated execution since
    // this can cause problems for nodes that do not copy output: ALL input and
    // connected param values must be new
    if(m == AllNew && !newInputAll()){
        debug(4) << __func__ << "Cannot enqueue node" << *this << ", some input is old";
        return;
    }
    //if(!newParamValues()){
    //    // ALL inputs must be new for slow nodes
    //    if(!newInputAll() && m_speed < medium){
    //        debug(2) << "Cannot enqueue node" << *this << ", some input is old";
    //        return;
    //    }else if(!newInput()){
    //        debug(2) << "Cannot enqueue node" << *this << ", all input is old";
    //        return;
    //    }
    //}

    debug(4) << __func__ << "Queuing node (" << m << "):" << *this;
    setExecQueued();
    m_sched.addJob(shared_from_this(), m_priority);
}

void Node::sendMessage(boost::shared_ptr<Message const> m, service_t p) const {
    m_pl.sendMessage(m, p);
}

/* Keep a record of which inputs are new (have changed since they were
 * last used by this node)
 * Check to see if this node should add itself to the scheduler queue
 */
void Node::setNewInput(input_id const& a){
    // new input implies it's probably valid.. if not we'll just find out the
    // next time we come to exec()
    setValidInput(a);
    unique_lock_t l(m_new_inputs_lock);
    unique_lock_t m(m_valid_inputs_lock);
    const in_bool_map_t::iterator i = m_new_inputs.find(a);
    debug(5) << *this << "setting input new" << a;
    if(i == m_new_inputs.end()){
        error e;
        e << a << "invalid, valid inputs:";
        foreach(in_bool_map_t::value_type const& v, m_new_inputs)
            e << v.second;
        throw id_error("newInput: Invalid input id: " + toStr(a));
    }else{
        i->second = true;
        m_valid_inputs[a] = true;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, a, NodeIOStatus::New | NodeIOStatus::Valid
        ));
    }
    m.unlock();
    l.unlock();
    checkAddSched();
}

/* mark all inputs as new
 */
void Node::setNewInput(){
    unique_lock_t m(m_new_inputs_lock);
    unique_lock_t n(m_valid_inputs_lock);
    debug(5) << *this << "setting all inputs new";
    foreach(in_bool_map_t::value_type& i, m_new_inputs){
        i.second = true;
        m_valid_inputs[i.first] = true;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, i.first, NodeIOStatus::New | NodeIOStatus::Valid
        ));
    }
    n.unlock();
    m.unlock();
    checkAddSched();
}

void Node::clearNewInput(){
    unique_lock_t m(m_new_inputs_lock);
    unique_lock_t n(m_valid_inputs_lock);
    debug(5) <<  *this << "setting all inputs old";
    foreach(in_bool_map_t::value_type& i, m_new_inputs){
        i.second = false;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, i.first,
            m_valid_inputs[i.first]? NodeIOStatus::Valid : NodeIOStatus::e(0)
        ));
    }
}

bool Node::newInputAll() const{
    shared_lock_t m(m_new_inputs_lock);
    shared_lock_t n(m_parent_links_lock);
    foreach(in_link_map_t::value_type const& i, m_parent_links)
        if(i.second.node && // only consider inputs (incl. params) that are connected
           m_new_inputs.count(i.first) &&
           !m_new_inputs.find(i.first)->second)
            return false;
    return true;
}

bool Node::newInput() const{
    shared_lock_t l(m_new_inputs_lock);
    shared_lock_t m(m_parent_links_lock);
    foreach(in_link_map_t::value_type const& i, m_parent_links)
        if(i.second.node && // only consider inputs (incl. params) that are connected
           m_new_inputs.count(i.first) &&
           m_new_inputs.find(i.first)->second)
            return true;
    return false;
}

void Node::setValidInput(input_id const& i){
    using namespace NodeIOStatus;
    unique_lock_t l(m_valid_inputs_lock);
    if(!m_valid_inputs[i]){
        unique_lock_t l(m_new_inputs_lock);
        m_valid_inputs[i] = true;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, i, m_new_inputs[i]? New | Valid : Valid
        ));
    }
    l.unlock();
    checkAddSched();
}

void Node::clearValidInput(input_id const& i){
    unique_lock_t l(m_valid_inputs_lock);
    if(m_valid_inputs[i]){
        unique_lock_t l(m_new_inputs_lock);
        m_valid_inputs[i] = false;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, i, m_new_inputs[i]? NodeIOStatus::New : NodeIOStatus::e(0)
        ));
    }
}

bool Node::validInputAll() const{
    //TODO SOON: better way of excluding parameters from this check
    // NB: for now, the order of locking here prevents deadlock if
    // setAllowQueue is called from paramChanged callback
    shared_lock_t n(m_parameters_lock);
    shared_lock_t l(m_valid_inputs_lock);
    foreach(in_bool_map_t::value_type const& v, m_valid_inputs)
        if(!v.second && !m_parameters.count(v.first))
            return false;
    return true;
}

/* This is called by the children of this node in order to request new
 * output. It may be called at the start or end of the child's exec()
 */
void Node::setNewOutputDemanded(output_id const& o){
    unique_lock_t l(m_output_demanded_lock);
    m_output_demanded_on.insert(o);
    if(!m_output_demanded){
        m_output_demanded = true;

        _statusMessage(boost::make_shared<OutputStatusMessage>(
            m_id, o, NodeIOStatus::Demanded
        ));

        unique_lock_t m(m_new_inputs_lock);
        unique_lock_t n(m_parent_links_lock);
        foreach(in_bool_map_t::value_type& v, m_new_inputs){
            if(!v.second && m_parent_links[v.first].node)
                m_parent_links[v.first].node->setNewOutputDemanded(
                    m_parent_links[v.first].id
                );
        }
        m.unlock();
        n.unlock();
    }
    l.unlock();
    checkAddSched();
}

void Node::clearNewOutputDemanded(output_id const& o){
    unique_lock_t l(m_output_demanded_lock);
    m_output_demanded_on.erase(o);
    m_output_demanded = !!m_output_demanded_on.size();
    _statusMessage(boost::make_shared<OutputStatusMessage>(
            m_id, o, NodeIOStatus::e(0)
    ));
}

bool Node::newOutputDemanded() const{
    if(this->isOutputNode())
        return true;
    shared_lock_t l(m_output_demanded_lock);
    return m_output_demanded;
}

void Node::setAllowQueue(){
    unique_lock_t l(m_allow_queue_lock);
    m_allow_queue = true;
    NodeStatus::e status = NodeStatus::AllowQueue;
    if(execQueued()) status |= NodeStatus::ExecQueued;
    _statusMessage(boost::make_shared<StatusMessage>(m_id, status));
    l.unlock();
    checkAddSched();
}

void Node::clearAllowQueue(){
    unique_lock_t l(m_allow_queue_lock);
    m_allow_queue = false;
    NodeStatus::e status = NodeStatus::e(0);
    if(execQueued()) status |= NodeStatus::ExecQueued;
    _statusMessage(boost::make_shared<StatusMessage>(m_id, status));
}

bool Node::allowQueue() const{
    shared_lock_t l(m_allow_queue_lock);
    return m_allow_queue;
}

void Node::setExecQueued(){
    unique_lock_t l(m_exec_queued_lock);
    m_exec_queued = true;
    NodeStatus::e status = NodeStatus::ExecQueued;
    if(allowQueue()) status |= NodeStatus::AllowQueue;
    _statusMessage(boost::make_shared<StatusMessage>(m_id, status));
}

void Node::clearExecQueued(){
    unique_lock_t l(m_exec_queued_lock);
    m_exec_queued = false;
    NodeStatus::e status = NodeStatus::e(0);
    if(allowQueue()) status |= NodeStatus::AllowQueue;
    _statusMessage(boost::make_shared<StatusMessage>(m_id, status));
    l.unlock();
    checkAddSched();
}

bool Node::execQueued() const{
    shared_lock_t l(m_exec_queued_lock);
    return m_exec_queued;
}

void Node::setNewParamValue(param_id const& a){
    unique_lock_t l(m_new_paramvalues_lock);
    const param_bool_map_t::iterator i = m_new_paramvalues.find(a);

    debug(5) << *this << "setting paramvalue new" << a;
    if(i == m_new_paramvalues.end()){
        error e;
        e << '"' << a << '"' << "invalid, valid paramvalues:\n\t";
        foreach(param_bool_map_t::value_type const& v, m_new_paramvalues)
            e << v.second << "\n\t";
        throw id_error("setNewParamValue: Invalid parameter id: " + toStr(a));
    }else{
        i->second = true;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, a, NodeIOStatus::New | NodeIOStatus::Valid
        ));
    }
    l.unlock();
    checkAddSched();
}

void Node::clearNewParamValues(){
    unique_lock_t m(m_new_paramvalues_lock);
    debug(5) <<  *this << "setting all paramvalues old";
    foreach(param_bool_map_t::value_type& i, m_new_paramvalues){
        i.second = false;
        _statusMessage(boost::make_shared<InputStatusMessage>(
            m_id, i.first, NodeIOStatus::e(0)
        ));
    }
}

bool Node::newParamValues() const{
    shared_lock_t l(m_new_paramvalues_lock);
    foreach(param_bool_map_t::value_type const& i, m_new_paramvalues)
        if(i.second)
            return true;
    return false;
}

void Node::_demandNewParentInput() throw(){
    unique_lock_t l(m_parent_links_lock);
    debug(5) << "node" << *this << "demanding new output from all parents";
    foreach(in_link_map_t::value_type& i, m_parent_links)
        if(i.second.node)
            i.second.node->setNewOutputDemanded(i.second.id);
}

void Node::_statusMessage(boost::shared_ptr<Message const> m){
    #ifndef NO_NODE_IO_STATUS
    m_pl.sendMessage(m);
    #endif
}

node_id Node::_newID() throw(){
    static node_id id = 1;
    if(id == ~node_id(0)){
        error() << "run out of node IDs, starting to recycle";
        id = 1;
    }
    return id++;
}

