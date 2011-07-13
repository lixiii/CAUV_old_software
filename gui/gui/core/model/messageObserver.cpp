#include "messageObserver.h"

#include <vector>

#include <debug/cauv_debug.h>

#include <gui/core/model/messagegenerators.h>
#include <gui/core/model/model.h>

using namespace cauv;
using namespace cauv::gui;

template<>
std::string NameConversion::toName<MotorID::e>(MotorID::e id){
    switch (id){
    case MotorID::Prop:
        return "prop";
    case MotorID::HBow:
        return "hbow";
    case MotorID::VBow:
        return "vbow";
    case MotorID::HStern:
        return "hstern";
    case MotorID::VStern:
        return "vstern";
    default:
        return "unknown";
    }
}

template<>
std::string NameConversion::toName<CameraID::e>(CameraID::e id){
    switch (id){
    case CameraID::Forward:
        return "forward";
    case CameraID::Down:
        return "down";
    case CameraID::Sonar:
        return "sonar";
    case CameraID::File:
        return "file";
    default:
        return "unknown";
    }
}

template<>
std::string NameConversion::toName<Controller::e>(Controller::e id){
    switch (id){
    case Controller::Depth:
        return "depth";
    case Controller::Bearing:
        return "bearing";
    case Controller::Pitch:
        return "pitch";
    default:
        return "unknown";
    }
}

GuiMessageObserver::GuiMessageObserver(boost::shared_ptr<AUV> auv):
        m_auv(auv){
    qRegisterMetaType<boost::shared_ptr<Message> >("boost::shared_ptr<Message>");
}


GuiMessageObserver::~GuiMessageObserver() {
    debug() << "~GuiMessageObserver()";
}


void GuiMessageObserver::addGenerator(boost::shared_ptr<NodeBase> node, boost::shared_ptr<MessageGenerator> generator){
    if (m_generators[node]) return; // already has a generator

    info() << "Adding generator for " << node->nodeName();
    m_generators[node] = generator;
    if(!connect(generator.get(), SIGNAL(messageGenerated(boost::shared_ptr<Message>)),
            this, SIGNAL(messageGenerated(boost::shared_ptr<Message>))))
        error() << "Failed when connecting message forwarding signals";
}

void GuiMessageObserver::onMotorStateMessage(MotorStateMessage_ptr message) {
    info() << message;

    std::string name = NameConversion::toName(message->motorId());
    boost::shared_ptr<NumericNode> motor = m_auv->findOrCreate<GroupingNode>("motors")->findOrCreateMutable<NumericNode>(name);
    motor->update(message->speed());
    motor->setMax(127);
    motor->setMin(-127);

    addGenerator(motor, boost::make_shared<MotorMessageGenerator>(m_auv, motor, message->motorId()));
}

void GuiMessageObserver::onBearingAutopilotEnabledMessage(BearingAutopilotEnabledMessage_ptr message) {
    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<GroupingNode> autopilot = autopilots->findOrCreate<GroupingNode>("bearing");
    boost::shared_ptr<NumericNode> target = autopilot->findOrCreateMutable<NumericNode>("target");
    target->setMin(0);
    target->setMax(360);
    target->setWraps(true);
    target->setUnits("°");
    target->update(message->target());
    autopilots->findOrCreate<GroupingNode>("bearing")->findOrCreateMutable<NumericNode>("enabled")->update(message->enabled());

    addGenerator(autopilot, boost::make_shared<AutopilotMessageGenerator>(m_auv, autopilot));
}

void GuiMessageObserver::onBearingAutopilotParamsMessage(BearingAutopilotParamsMessage_ptr message) {
    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<GroupingNode> ap = autopilots->findOrCreate<GroupingNode>("bearing");

    ap->findOrCreateMutable<NumericNode>("Kp")->update(message->Kp());
    ap->findOrCreateMutable<NumericNode>("Ki")->update(message->Ki());
    ap->findOrCreateMutable<NumericNode>("Kd")->update(message->Kd());
    ap->findOrCreateMutable<NumericNode>("scale")->update(message->scale());
    ap->findOrCreateMutable<NumericNode>("aP")->update(message->Ad());
    ap->findOrCreateMutable<NumericNode>("aI")->update(message->Ai());
    ap->findOrCreateMutable<NumericNode>("aD")->update(message->Ad());
    ap->findOrCreateMutable<NumericNode>("thr")->update(message->thr());
    ap->findOrCreateMutable<NumericNode>("maxError")->update(message->maxError());
}

void GuiMessageObserver::onDepthAutopilotEnabledMessage(DepthAutopilotEnabledMessage_ptr message) {
    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<NumericNode> target = autopilots->findOrCreateMutable<GroupingNode>("depth")->findOrCreate<NumericNode>("target");
    target->setMin(0);
    target->setMax(5);
    target->setUnits("m");
    target->update(message->target());
    autopilots->findOrCreateMutable<GroupingNode>("depth")->findOrCreate<NumericNode>("enabled")->update(message->enabled());
}

void GuiMessageObserver::onDepthAutopilotParamsMessage(DepthAutopilotParamsMessage_ptr message) {
    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<GroupingNode> ap = autopilots->findOrCreate<GroupingNode>("depth");

    ap->findOrCreateMutable<NumericNode>("Kp")->update(message->Kp());
    ap->findOrCreateMutable<NumericNode>("Ki")->update(message->Ki());
    ap->findOrCreateMutable<NumericNode>("Kd")->update(message->Kd());
    ap->findOrCreateMutable<NumericNode>("scale")->update(message->scale());
    ap->findOrCreateMutable<NumericNode>("aP")->update(message->Ad());
    ap->findOrCreateMutable<NumericNode>("aI")->update(message->Ai());
    ap->findOrCreateMutable<NumericNode>("aD")->update(message->Ad());
    ap->findOrCreateMutable<NumericNode>("thr")->update(message->thr());
    ap->findOrCreateMutable<NumericNode>("maxError")->update(message->maxError());
}

void GuiMessageObserver::onPitchAutopilotEnabledMessage(PitchAutopilotEnabledMessage_ptr message) {
    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<NumericNode> target = autopilots->findOrCreateMutable<GroupingNode>("pitch")->findOrCreate<NumericNode>("target");
    target->setMin(-180);
    target->setMax(180);
    target->setUnits("°");
    target->update(message->target());
    autopilots->findOrCreateMutable<GroupingNode>("pitch")->findOrCreate<NumericNode>("enabled")->update(message->enabled());
}

void GuiMessageObserver::onPitchAutopilotParamsMessage(PitchAutopilotParamsMessage_ptr message) {
    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<GroupingNode> ap = autopilots->findOrCreate<GroupingNode>("pitch");

    ap->findOrCreateMutable<NumericNode>("Kp")->update(message->Kp());
    ap->findOrCreateMutable<NumericNode>("Ki")->update(message->Ki());
    ap->findOrCreateMutable<NumericNode>("Kd")->update(message->Kd());
    ap->findOrCreateMutable<NumericNode>("scale")->update(message->scale());
    ap->findOrCreateMutable<NumericNode>("aP")->update(message->Ad());
    ap->findOrCreateMutable<NumericNode>("aI")->update(message->Ai());
    ap->findOrCreateMutable<NumericNode>("aD")->update(message->Ad());
    ap->findOrCreateMutable<NumericNode>("thr")->update(message->thr());
    ap->findOrCreateMutable<NumericNode>("maxError")->update(message->maxError());
}


void GuiMessageObserver::onDepthCalibrationMessage(DepthCalibrationMessage_ptr message) {
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("sensors")->findOrCreate<GroupingNode>("calibration");
    group->findOrCreateMutable<NumericNode>("aftMultiplier")->update(message->aftMultiplier());
    group->findOrCreateMutable<NumericNode>("aftOffset")->update(message->aftOffset());
    group->findOrCreateMutable<NumericNode>("foreMultiplier")->update(message->foreMultiplier());
    group->findOrCreateMutable<NumericNode>("foreOffset")->update(message->foreOffset());
}

void GuiMessageObserver::onDebugLevelMessage(DebugLevelMessage_ptr message) {
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("debug");
    group->findOrCreateMutable<NumericNode>("level")->update(message->level());
}

void GuiMessageObserver::onImageMessage(ImageMessage_ptr message) {
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("image");
    boost::shared_ptr<Image> shared_image= boost::make_shared<Image>(message->image());
    group->findOrCreate<ImageNode>(NameConversion::toName(message->source()))->update(shared_image);
}

void GuiMessageObserver::onSonarControlMessage(SonarControlMessage_ptr message) {
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("image");
    boost::shared_ptr<ImageNode> sonar = group->findOrCreate<ImageNode>("sonar");
    sonar->findOrCreateMutable<NumericNode>("direction")->update(message->direction());
    sonar->findOrCreateMutable<NumericNode>("width")->update(message->width());
    sonar->findOrCreateMutable<NumericNode>("gain")->update(message->gain());
    sonar->findOrCreateMutable<NumericNode>("range")->update(message->range());
    sonar->findOrCreateMutable<NumericNode>("rangeRes")->update(message->rangeRes());
    sonar->findOrCreateMutable<NumericNode>("angularRes")->update(message->angularRes());
}

void GuiMessageObserver::onTelemetryMessage(TelemetryMessage_ptr message){
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("telemtry");
    group->findOrCreate<NumericNode>("depth")->update(message->depth());
    group->findOrCreate<FloatYPRNode>("orientation")->update(message->orientation());

    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    autopilots->findOrCreate<GroupingNode>("bearing")->findOrCreate<NumericNode>("actual")->update(message->orientation().yaw);
    autopilots->findOrCreate<GroupingNode>("pitch")->findOrCreate<NumericNode>("actual")->update(message->orientation().pitch);
    autopilots->findOrCreate<GroupingNode>("depth")->findOrCreate<NumericNode>("actual")->update(message->depth());
}

void GuiMessageObserver::onLocationMessage(LocationMessage_ptr m){
    //boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("telemetry")->findOrCreate<GroupingNode>("location");

    //group->findOrCreate<NumericNode>("speed")->update(message->aft());
    //group->findOrCreate<NumericNode>("")->update(message->aft());
    //m_auv->sensors.speed->update(m->speed());
    //m_auv->sensors.location->update(*(m.get()));
}

void GuiMessageObserver::onPressureMessage(PressureMessage_ptr message){
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("sensors")->findOrCreate<GroupingNode>("pressure");
    group->findOrCreate<NumericNode>("fore")->update(message->fore());
    group->findOrCreate<NumericNode>("aft")->update(message->aft());
}

void GuiMessageObserver::onScriptResponseMessage(ScriptResponseMessage_ptr message){
//    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("console");
//    group->findOrCreate<StringNode>("response")->update(message->response());
}

void GuiMessageObserver::onBatteryUseMessage(BatteryUseMessage_ptr message) {
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("telemetry")->findOrCreate<GroupingNode>("battery");
    group->findOrCreate<NumericNode>("current")->update(message->estimate_current());
    group->findOrCreate<NumericNode>("total")->update(message->estimate_total());
    group->findOrCreate<NumericNode>("remaining")->update(message->fraction_remaining() * 100.f);
}

void GuiMessageObserver::onProcessStatusMessage(ProcessStatusMessage_ptr message) {
    boost::shared_ptr<GroupingNode> group = m_auv->findOrCreate<GroupingNode>("processes");
    boost::shared_ptr<GroupingNode> process = group->findOrCreate<GroupingNode>(message->process());

    process->findOrCreate<NumericNode>("cpu")->update(message->cpu());
    process->findOrCreate<NumericNode>("mem")->update(message->mem());
    process->findOrCreate<NumericNode>("threads")->update(message->threads());
    process->findOrCreate<StringNode>("status")->update(message->status());
}

void GuiMessageObserver::onControllerStateMessage(ControllerStateMessage_ptr message){

    std::string controller = NameConversion::toName(message->contoller());

    boost::shared_ptr<GroupingNode> autopilots = m_auv->findOrCreate<GroupingNode>("autopilots");
    boost::shared_ptr<GroupingNode> ap = autopilots->findOrCreate<GroupingNode>(controller);
    boost::shared_ptr<GroupingNode> state = ap->findOrCreate<GroupingNode>(controller);

    state->findOrCreate<NumericNode>("Kp")->update(message->kp());
    state->findOrCreate<NumericNode>("Ki")->update(message->ki());
    state->findOrCreate<NumericNode>("Kd")->update(message->kd());
    boost::shared_ptr<GroupingNode> demands = state->findOrCreate<GroupingNode>("demands");
    demands->findOrCreate<NumericNode>(NameConversion::toName(MotorID::Prop))->update(message->demand().prop);
    demands->findOrCreate<NumericNode>(NameConversion::toName(MotorID::HBow))->update(message->demand().hbow);
    demands->findOrCreate<NumericNode>(NameConversion::toName(MotorID::HStern))->update(message->demand().hstern);
    demands->findOrCreate<NumericNode>(NameConversion::toName(MotorID::VBow))->update(message->demand().vbow);
    demands->findOrCreate<NumericNode>(NameConversion::toName(MotorID::VStern))->update(message->demand().vstern);
    state->findOrCreate<NumericNode>("error")->update(message->error());
    state->findOrCreate<NumericNode>("derror")->update(message->derror());
    state->findOrCreate<NumericNode>("ierror")->update(message->ierror());
    state->findOrCreate<NumericNode>("mv")->update(message->mv());
}
