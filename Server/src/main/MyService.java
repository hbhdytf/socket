
package main;

import java.io.IOException;  
import java.net.ServerSocket;  
import java.net.Socket;  
import java.util.ArrayList;  
import java.util.List;  
  
public class MyService {  
  
    // ���屣�����е�Socket  
    public static List<Socket> socketList = new ArrayList<Socket>();  
  
    public static void main(String[] args) throws IOException {  
        ServerSocket server = new ServerSocket(3000);  
        System.out.print("Server begin!\n");
        while(true){  
            Socket s=server.accept();  
            socketList.add(s);  
            //ÿ���ͻ�������֮������һ��ServerThread�߳�Ϊ�ÿͻ��˷���  
            new Thread(new ServiceThreada(s)).start();  
              
        }  
    }  
  
}  