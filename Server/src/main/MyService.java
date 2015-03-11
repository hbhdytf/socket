
package main;

import java.io.IOException;  
import java.net.ServerSocket;  
import java.net.Socket;  
import java.util.ArrayList;  
import java.util.List;  
  
public class MyService {  
  
    // 定义保存所有的Socket  
    public static List<Socket> socketList = new ArrayList<Socket>();  
  
    public static void main(String[] args) throws IOException {  
        ServerSocket server = new ServerSocket(3000);  
        System.out.print("Server begin!\n");
        while(true){  
            Socket s=server.accept();  
            socketList.add(s);  
            //每当客户端连接之后启动一条ServerThread线程为该客户端服务  
            new Thread(new ServiceThreada(s)).start();  
              
        }  
    }  
  
}  