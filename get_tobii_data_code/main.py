import json
import os
import signal
import socketserver
import sys
import threading
import time
from queue import Queue
import utils.commons


MARGIN_BIAS = 40
server = None
valid_resolution = (1920, 1200)
gaze_screen = (0, 0)

syncq = Queue(60)


class MyTCPHandler(socketserver.BaseRequestHandler):
    def __init__(self, *args, **kwargs):
        print("init MyTCPHandler")
        self.last_data = b''
        self.state_manager = None
        super().__init__(*args, **kwargs)

    # part of the system
    def setup(self):
        print(f"ESTABLISHED CONNECTION [{self.client_address[0]}]")
        pass

    def handle(self):
        print("start MyTCPHandler")
        while True:
            # unpack received message
            try:
                self.last_data += self.request.recv(1024)
                data = self.last_data.split(b'\n')
                self.last_data = data[-1]
            except ConnectionResetError:
                print(f"CONNECTION RESET [{self.client_address[0]}]")
                break
            packets = data[:-1]

            mydata = ""
            for packet in packets:
                try:
                    mydata = json.loads(packet)
                except json.decoder.JSONDecodeError:
                    print("Tobii Decoder Error.")
                    pass
            try:
                pass
            except TypeError:
                print(f"TYPE ERROR [{self.client_address[0]}]")
                break

            # put into message queue
            rawX = mydata["RawX"]
            rawY = mydata["RawY"]
            screenX = mydata["ScreenX"]
            screenY = mydata["ScreenY"]

            print(screenX, screenY)
            timestamp = time.time()

            global gaze_screen, syncq
            gaze_screen = (screenX, screenY)
            syncq.put((screenX, screenY, timestamp))
            # syncq.put((screenX, screenY))

    def finish(self):
        print(f"FINISHED CONNECTION [{self.client_address[0]}]")


def server_thread():
    global server
    server = socketserver.ThreadingTCPServer((utils.commons.HOST, utils.commons.PORT), MyTCPHandler)
    server.serve_forever()
    print("server thread started")


def fork_gaze_exe():
    gaze_point_exe = os.path.join(os.curdir, 'lib', "MCeyegazethesisNET461")
    try:
        os.startfile(gaze_point_exe)
    except FileNotFoundError:
        print(f"MCeyegazethesisNET461.exe not found at {gaze_point_exe}")


def write_data():
    global syncq
    log_file_prefix = f"log/exp_{utils.commons.exp_num}"
    if not os.path.exists(log_file_prefix):
        os.makedirs(log_file_prefix)

    log_file = None

    try:
        while True:
            if not syncq.empty():
                data = syncq.get()
                log_file = open(f"{log_file_prefix}/tobii_log.txt", 'a')
                log_file.write(f"{data[0]}, {data[1]}, {data[2]}\n")
                log_file.close()
    except Exception as e:
        print(e)
    finally:
        if log_file:
            log_file.close()
        print("log file closed")


if __name__ == '__main__':
    threading.Thread(target=write_data).start()
    threading.Thread(target=server_thread).start()
    threading.Thread(target=fork_gaze_exe).start()
