package cauv.messaging;

import java.util.LinkedList;
import java.util.Map;
import java.util.Vector;
import java.util.HashMap;
import java.io.*;

import cauv.types.*;
import cauv.utils.*;

public class AliveMessage extends Message {
    int m_id = 40;

    private byte[] bytes;


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


            return bs.toByteArray();
        }
    }

    public AliveMessage(){
        super(40, "mcb");
        this.bytes = null;
    }


    public AliveMessage(byte[] bytes) {
        super(40, "mcb");
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
                    throw new IllegalArgumentException("Attempted to create AliveMessage with invalid id");
                }


                bytes = null;
            }
        }
        catch (IOException e) {}
    }
}
