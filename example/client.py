import sys
# use absolute path to import rpc component
sys.path.append("D:/codetest/LittleRpc/src/python") # windows
# sys.path.append("/home/user/codetest/LittleRpc/src/python") # linux
from little_rpc import *

class MyClient(RpcClientCallback):
    def __init__(self):
        self.client = RpcClient()
        self.client.set_call_back(self)

    # inherited callbacks
    def on_connected(self):
        print ("on_connected")

    def on_disconnected(self):
        print ("on_disconnected")

    def on_msg_result(self, msg):
        # here you can dispatch msg with self defined call_id
        print ("on_msg_result: " + msg.decode("utf8")) # must convert bytes to str

    def connect_server(self, addr):
        # FIXME: need sync check connect status
        self.client.connect(addr) 

    def send_request(self, data):
        self.client.send(data)

if __name__ == "__main__":

    addr = "tcp://127.0.0.1:5000";

    my_client = MyClient()

    # connect
    my_client.connect_server(addr)
    print ("connect to ", addr)

    # test send data
    my_client.send_request("request for happy life")
    print ("send to server")

    # avoid exit main thread
    while True:
        time.sleep(1)