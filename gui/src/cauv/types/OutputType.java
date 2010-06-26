package cauv.types;

import java.util.LinkedList;
import java.util.Vector;
import java.util.HashMap;
import java.io.*;

import cauv.utils.*; 

public enum OutputType {

    Image, Parameter;

    public static OutputType readFrom(LEDataInputStream s) throws IOException {
        byte val = s.readByte();
        switch (val) {
            case 0:
                return Image;
            case 1:
                return Parameter;
            default:
                throw new IllegalArgumentException("Unrecognized OutputType value: " + val);
        }
    }

    public void writeInto(LEDataOutputStream s) throws IOException {
        switch (this) {
            case Image:
                s.writeByte(0);
                break;
            case Parameter:
                s.writeByte(1);
                break;
        }
    }
}


