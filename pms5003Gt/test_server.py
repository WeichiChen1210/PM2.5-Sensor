import socket
import threading

# Request handler
def handler(sock,addr):
    msg = 'YEE from server. YEEEEEEEEEE'
    print('Connection established')
    sock.send(msg.encode('utf-8'))

    while True:
        msg=sock.recv(1024)
        if not msg:
            print("ERROR : No data")
        else:
            print(msg.decode('utf-8'))
            msg = 'Data received'
            sock.send(msg.encode('utf-8'))
   
    msg = 'Closing connection...'
    sock.send(msg.encode('utf-8'))
    sock.close()
if __name__ == '__main__':
    # Turn on server
    sock = socket.socket()
    sock.bind(('0.0.0.0', 82))# port
    sock.listen(5)
    
    print('Waiting for connection...')
    while True:
        (socket,addr) = sock.accept()
        # Create a new thread to handle requests
        thread = threading.Thread(target=handler,args=(socket,addr))
        thread.start()
