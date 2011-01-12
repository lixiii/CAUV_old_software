
#include "datastreamdragging.h"

#include <debug/cauv_debug.h>

using namespace cauv;

/*
template<> void DataStreamTreeItem<int8_t>::onChange(const int8_t value){
    std::stringstream stream;
    stream << (int)value;
    this->setText(1, QString::fromStdString(stream.str()));
}
*/

void DataStreamDropListener::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void DataStreamDropListener::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    DataStreamDragSource * source = dynamic_cast<DataStreamDragSource*> (event->source());
    if(source) {
        info() << "Drag receieved from " << source;

        boost::shared_ptr<std::vector<boost::shared_ptr<DataStreamBase> > >  streams = source->getDataStreams();
        std::vector<boost::shared_ptr<DataStreamBase> >::iterator it;
        for (it = streams->begin(); it!=streams->end(); ++it) {
            routeStream(*it);
        }
    }
}

bool DataStreamDropListener::routeStream(boost::shared_ptr<DataStreamBase> s){

    if(dynamic_cast<DataStream<int8_t> *>(s.get())) {
        info() << s->getName() << " - int8_t stream dropped";
        onStreamDropped(boost::static_pointer_cast<DataStream<int8_t> >(s));
    }
    else if(dynamic_cast<DataStream<int> *>(s.get())) {
        info() << s->getName() << " - int stream dropped";
        onStreamDropped(boost::static_pointer_cast<DataStream<int> >(s));
    }
    else if(dynamic_cast<DataStream<float> *>(s.get())) {
        info() << s->getName() << " - float stream dropped";
        onStreamDropped(boost::static_pointer_cast<DataStream<float> >(s));
    }
    else if(dynamic_cast<DataStream<uint16_t> *>(s.get())) {
        info() << s->getName() << " - uint16_t stream dropped";
        onStreamDropped(boost::static_pointer_cast<DataStream<uint16_t> >(s));
    }
    else if(dynamic_cast<DataStream<autopilot_params_t> *>(s.get())){
        info() << s->getName() << " - autopilot_params_t stream dropped";
        onStreamDropped(boost::static_pointer_cast<DataStream<autopilot_params_t> >(s));
    }
    else if(dynamic_cast<DataStream<floatYPR> *>(s.get())){
        info() << s->getName() << " - floatYPR stream dropped";
        onStreamDropped(boost::static_pointer_cast<DataStream<floatYPR> >(s));
    }
    else return false;

    return true;
}


void DataStreamDropListener::onStreamDropped(boost::shared_ptr<DataStream<int8_t> > ){

}

void DataStreamDropListener::onStreamDropped(boost::shared_ptr<DataStream<int> > ){

}

void DataStreamDropListener::onStreamDropped(boost::shared_ptr<DataStream<float> > ){

}

void DataStreamDropListener::onStreamDropped(boost::shared_ptr<DataStream<autopilot_params_t> > ){

}

void DataStreamDropListener::onStreamDropped(boost::shared_ptr<DataStream<floatYPR> > ){

}

void DataStreamDropListener::onStreamDropped(boost::shared_ptr<DataStream<uint16_t> > ){

}
