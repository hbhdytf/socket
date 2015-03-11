package main;

import java.io.BufferedReader;  
import java.io.IOException;  
import java.io.InputStreamReader;  
import java.io.OutputStream;  
import java.net.Socket;  
  
public class ServiceThreada implements Runnable {  
  
    // ���嵱ǰ�̴߳�����Socket  
    Socket s = null;  
    // ���߳���������Socket����Ӧ��������  
    BufferedReader br = null;  
  
    public ServiceThreada(Socket s) {  
        this.s = s;  
        try {  
            br = new BufferedReader(new InputStreamReader(s.getInputStream()));  
        } catch (IOException e) {  
            e.printStackTrace();  
        }  
    }  
  
    public void run() {  
  
        String content = null;  
        //����ѭ�����ϵĴ�Socket�ж�ȡ�ͻ��˷��͹���������  
        while((content=readFromClient())!=null){  
            //����socketList�е�ÿ��Socket  
            //����ȡ��������ÿ����Socket����һ��  
            for(Socket s:MyService.socketList){  
                OutputStream os;  
                try {  
                    os = s.getOutputStream();  
                    os.write((content+"\n").getBytes("gbk"));  
                } catch (IOException e) {  
                    // TODO Auto-generated catch block  
                    e.printStackTrace();  
                }  
                  
            }  
        }  
  
    }  
  
    // �����ȡ�ͻ��˵���Ϣ  
    public String readFromClient() {  
        try {  
            return br.readLine();  
        } catch (Exception e) {  
            e.printStackTrace();  
        }  
        return null;  
    }  
  
}  