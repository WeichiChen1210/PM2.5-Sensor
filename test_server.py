import socket
import threading
import datetime
import json

# Request handler
def handler(sock,addr):
    msg = f'{datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\0'
    print('Connection established')
    sock.send(msg.encode('utf-8'))

    while True:
        msg=sock.recv(1024)
        if not msg:
            print("ERROR : No data")
        else:
            # data = json.loads(msg.decode('utf-8').replace("\'", "\""))
            print(f"{msg}")
   
    msg = 'Closing connection...'
    sock.send(msg.encode('utf-8'))
    sock.close()
if __name__ == '__main__':
    # Turn on server
    sock = socket.socket()
    sock.bind(('0.0.0.0', 8080))# port
    sock.listen(5)
    
    print('Waiting for connection...')
    while True:
        (socket,addr) = sock.accept()
        # Create a new thread to handle requests
        thread = threading.Thread(target=handler,args=(socket,addr))
        thread.start()
