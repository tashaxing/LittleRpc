# only contain the client side python rpc component, the server side should use C++

import sys
import threading
import time
import pandas as pd
import zmq
import json

class RpcClientCallback:
    def __init__(self):
        pass
    def on_connected(self):
        pass
    def on_disconnected(self):
        pass
    def on_msg_result(msg):
        pass

class RpcClient:
    def __init__(self):
        context = zmq.Context()
        self.__client_sock = context.socket(zmq.DEALER)
        self.__addr = ""
        self.__conn_id = ""
        self.__msg_thread = None
        self.__should_close = False
        self.__callback = None

    def set_call_back(self, callback):
        self.__callback = callback

    def connect(self, addr):
        self.__addr = addr

        # build unique identity with time stamp, accurate only to 1 ms
        now = time.time()
        self.__conn_id = "py_client_" + str(now) 

        # goto main msg loop
        self.__msg_thread = threading.Thread(target=self.__run, name="run")
        self.__msg_thread.start() # FIXME: when to join?

    def __run(self):
        identity = self.__conn_id
        self.__client_sock.identity = identity.encode("utf8")
        self.__client_sock.connect(self.__addr)

        poll = zmq.Poller()
        poll.register(self.__client_sock, zmq.POLLIN)
        try:
            while not self.__should_close:
                sockets = dict(poll.poll(10))
                if self.__client_sock in sockets:
                    self.__receive()

        except Exception as e:
            print ("exception:", e)
        finally:
            pass

    def __receive(self):
        msg = self.__client_sock.recv()
        if not self.__callback is None:
            self.__callback.on_msg_result(msg)

    def send(self, data):
        self.__client_sock.send_string(data)