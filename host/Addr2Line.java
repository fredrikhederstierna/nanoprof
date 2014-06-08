
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.HashMap;





//-------------------------------


import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Represents a stack call. This is used to return all of the call
 * information as one object.
 */
final class NativeStackCallInfo {
    private final static Pattern SOURCE_NAME_PATTERN = Pattern.compile("^(.+):(\\d+)$");
    
    /** name of the library */
    private String mLibrary;

    /** name of the method */
    private String mMethod;

    /**
     * name of the source file + line number in the format<br>
     * &lt;sourcefile&gt;:&lt;linenumber&gt;
     */
    private String mSourceFile;
    
    private int mLineNumber = -1; 

    /**
     * Basic constructor with library, method, and sourcefile information
     *
     * @param lib The name of the library
     * @param method the name of the method
     * @param sourceFile the name of the source file and the line number
     * as "[sourcefile]:[fileNumber]"
     */
    public NativeStackCallInfo(String lib, String method, String sourceFile) {
        mLibrary = lib;
        mMethod = method;
        
        Matcher m = SOURCE_NAME_PATTERN.matcher(sourceFile);
        if (m.matches()) {
            mSourceFile = m.group(1);
            try {
                mLineNumber = Integer.parseInt(m.group(2));
            } catch (NumberFormatException e) {
                // do nothing, the line number will stay at -1
            }
        } else {
            mSourceFile = sourceFile;
        }
    }
    
    /**
     * Returns the name of the library name.
     */
    public String getLibraryName() {
        return mLibrary;
    }

    /** 
     * Returns the name of the method.
     */
    public String getMethodName() {
        return mMethod;
    }
    
    /**
     * Returns the name of the source file.
     */
    public String getSourceFile() {
        return mSourceFile;
    }

    /**
     * Returns the line number, or -1 if unknown.
     */
    public int getLineNumber() {
        return mLineNumber;
    }
}


//-------------------------------

public class Addr2Line {

  /** Path to the library */
  private String mLibrary;
  
  /** the command line process */
  private Process mProcess;
  
  /** buffer to read the result of the command line process from */
  private BufferedReader mResultReader;
  
  /**
   * output stream to provide new addresses to decode to the command line
   * process
   */
  private BufferedOutputStream mAddressWriter;
  
  
  /**
   * byte array representing a carriage return. Used to push addresses in the
   * process pipes.
   */
  private static final byte[] sCrLf = {
    '\n'
  };


  public static Addr2Line getProcess(final String library) {
    Addr2Line process = null;
    // synchronize around the hashmap object
    if (library != null) {
      process = new Addr2Line(library);
      // then we start it
      boolean status = process.start();
      if (! status) {
        // otherwise we just drop the object, to return null
        process = null;
      }
    }
    // return the process
    return process;
  }


  private Addr2Line(final String library) {
    mLibrary = library;
  }






 /**
  * Starts the command line process.
  *
  * @return true if the process was started, false if it failed to start, or
  *         if there was any other errors.
  */
  private boolean start() {

    // build the command line
    String[] command = new String[5];
    command[0] = "/usr/local/gcc/arm-eabi-tools-4.9.0/bin/arm-none-eabi-addr2line";
    command[1] = "-C";
    command[2] = "-f";
    command[3] = "-e";
    command[4] = "/home/fredrik/github/build/example/example.elf";

    try {
      // attempt to start the process
      mProcess = Runtime.getRuntime().exec(command);
      if (mProcess != null) {
        // get the result reader
        InputStreamReader is = new InputStreamReader(mProcess.getInputStream());
        mResultReader = new BufferedReader(is);
        // get the outstream to write the addresses
        mAddressWriter = new BufferedOutputStream(mProcess.getOutputStream());
        // check our streams are here
        if (mResultReader == null || mAddressWriter == null) {
          // not here? stop the process and return false;
          mProcess.destroy();
          mProcess = null;
          return false;
        }
        // return a success
        return true;
      }
    } catch (IOException e) {
      // log the error
      String msg = String.format("Error while trying to start %1$s process for library %2$s", command[0], mLibrary);
      System.out.println(msg);
      // drop the process just in case
      if (mProcess != null) {
        mProcess.destroy();
        mProcess = null;
      }
    }
    // we can be here either cause the allocation of mProcess failed, or we
    // caught an exception
    return false;
  }

  /**
   * Stops the command line process.
   */
  public void stop() {

    if (mProcess != null) {

      // then stops the process
      mProcess.destroy();
      // set the reference to null.
      // this allows to make sure another thread calling getAddress()
      // will not query a stopped thread
      mProcess = null;
    }
    
  }


  /**
   * Looks up an address and returns method name, source file name, and line
   * number.
   *
   * @param addr the address to look up
   * @return a BacktraceInfo object containing the method/filename/linenumber
   *         or null if the process we stopped before the query could be
   *         processed, or if an IO exception happened.
   */
  public NativeStackCallInfo getAddress(long addr) {
    // even though we don't access the hashmap object, we need to
    // synchronized on it to prevent
    // another thread from stopping the process we're going to query.

      // check the process is still alive/allocated
      if (mProcess != null) {
        // prepare to the write the address to the output buffer.
        // first, conversion to a string containing the hex value.
        String tmp = Long.toString(addr, 16);
        try {
          // write the address to the buffer
          mAddressWriter.write(tmp.getBytes());
          // add CR-LF
          mAddressWriter.write(sCrLf);
          // flush it all.
          mAddressWriter.flush();
          // read the result. We need to read 2 lines
          String method = mResultReader.readLine();
          String source = mResultReader.readLine();
          // make the backtrace object and return it
          if (method != null && source != null) {
            return new NativeStackCallInfo(mLibrary, method, source);
          }
        } catch (IOException e) {
          // log the error
          System.out.println("Error while trying to get information for addr: "
                + tmp + " in library: " + mLibrary);
          // we'll return null later
        }
      }

    return null;
  }
  

  public static void main(String[] args) {
    Addr2Line a2l = Addr2Line.getProcess("lib-hej.so");
    NativeStackCallInfo nsci = a2l.getAddress(0x080002c4);
    System.out.println("Lib:" + nsci.getLibraryName());
    System.out.println("Meth:" + nsci.getMethodName());
    System.out.println("Src:" + nsci.getSourceFile());
    System.out.println("Line:" + nsci.getLineNumber());
  }


}
