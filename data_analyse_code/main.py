import os
import time

import cv2
import numpy as np
from matplotlib.animation import FuncAnimation, FFMpegWriter, ImageMagickFileWriter
import matplotlib.pyplot as plt
import pyautogui


global ani


def update_video(frame, gaze_data_list, circle_1, circle_2):
    x = gaze_data_list[frame][0]
    y = gaze_data_list[frame][1]
    print(frame)
    circle_1.set_center((x, y))
    circle_2.set_center((x, y))
    return circle_1, circle_2


def convert_tobii_log_to_video(exp_name):
    screen_top_left = (831, 368)
    screen_bottom_right = (1089, 930)

    log_file = f"log/{exp_name}/tobii_log.txt"

    gaze_data_list = []
    with open(log_file, "r") as f:
        lines = f.readlines()
        for line in lines:
            if line == "\n":
                continue
            x = int(line.split(", ")[0])
            y = int(line.split(", ")[1])
            event_time = float(line.split(", ")[2])
            gaze_data_list.append((x, y, event_time))

    # create a 29.97 fps video by add a circle to a blank image
    fig, ax = plt.subplots(figsize=(5, 10))
    # 去掉ax周围的刻度
    ax.set_xticks([])
    ax.set_yticks([])

    plt.subplots_adjust(left=0.05, right=0.95, top=0.95, bottom=0.05)
    ax.set_xlim(screen_top_left[0], screen_bottom_right[0])
    ax.set_ylim(screen_bottom_right[1], screen_top_left[1])
    ax.set_aspect('equal')

    circle_1 = plt.Circle((gaze_data_list[0], gaze_data_list[1]), 12, color="black")
    circle_2 = plt.Circle((gaze_data_list[0], gaze_data_list[1]), 8, color="red")
    ax.add_artist(circle_1)
    ax.add_artist(circle_2)

    for index in range(len(gaze_data_list)):
        print(index)
        x = gaze_data_list[index][0]
        y = gaze_data_list[index][1]
        circle_1.set_center((x, y))
        circle_2.set_center((x, y))
        file_prefix = f"video/{exp_name}/tobii_images"
        if not os.path.exists(file_prefix):
            os.makedirs(file_prefix)
        plt.savefig(f"{file_prefix}/{index}.png", dpi=300)

    # ani = FuncAnimation(fig, update_video, frames=len(gaze_data_list), fargs=(gaze_data_list, circle_1, circle_2), interval=1000/29.97, blit=True)
    # # plt.show()
    # writer = ImageMagickFileWriter(fps=29.97, bitrate=1800)
    # ani.save(f"video/{exp_name}/tobii_log.mp4", writer=writer)
    # plt.close(fig)


if __name__ == "__main__":
    convert_tobii_log_to_video("exp_2")

