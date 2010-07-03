package cauv.messaging;

import java.util.LinkedList;
import java.util.Map;
import java.util.Vector;
import java.util.HashMap;
import java.io.*;

import cauv.types.*;
import cauv.utils.*;

public class DepthMessage extends Message {
    int m_id = 51;
    protected float depth;

    private byte[] bytes;

    public void depth(float depth) {
        deserialise();
        this.depth = depth;
    }
    public float depth() {
        deserialise();
        return this.depth;
    }


    public byte[] toBytes() throws IOException {
        if (bytes != null)
        {
            return bytes;
        }
        else
        {
            ByteArrayOutputStream bs = new ByteArrayOutputStream();
            LEDataOutputStream s = new LEDataOutputStream(bs);
            s.writeInt(m_id);

            s.writeFloat(this.depth);

            return bs.toByteArray();
        }
    }

    public DepthMessage(){
        super(51, "telemetry");
        this.bytes = null;
    }

    public DepthMessage(Float depth) {
        super(51, "telemetry");
        this.bytes = null;

        this.depth = depth;
    }

    public DepthMessage(byte[] bytes) {
        super(51, "telemetry");
        this.bytes = bytes;
    }

    public void deserialise() {
        try { 
            if (bytes != null)
            {
                ByteArrayInputStream bs = new ByteArrayInputStream(bytes);
                LEDataInputStream s = new LEDataInputStream(bs);
                int buf_id = s.readInt();
                if (buf_id != m_id)
                {
                    throw new IllegalArgumentException("Attempted to create DepthMessage with invalid id");
                }

                this.depth = s.readFloat();

                bytes = null;
            }
        }
        catch (IOException e) {}
    }
}