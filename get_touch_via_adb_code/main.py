import json
import logging
import os
import re
import subprocess

import configs


def add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot):
    if not (position_tobe_added[0] == -1 and position_tobe_added[1] == -1) and (position_tobe_added[0] == -1 or position_tobe_added[1] == -1):
        empty_index = 0 if position_tobe_added[0] == -1 else 1
        if len(slot_dict[current_slot]) > 0:
            position_tobe_added[empty_index] = slot_dict[current_slot][-1][empty_index]
            slot_dict[current_slot].append(position_tobe_added)
        else:
            raise Exception("Invalid format")
    elif position_tobe_added[0] != -1 and position_tobe_added[1] != -1:
        slot_dict[current_slot].append(position_tobe_added)


def get_screen_resolution():
    # 使用 adb shell wm size 获取屏幕分辨率
    result = subprocess.run(['adb', 'shell', 'wm', 'size'], capture_output=True, text=True)
    output = result.stdout.strip()
    width, height = map(int, output.split()[2].split('x'))
    return width, height


def get_abs_mt_position():
    result = subprocess.run(['adb', 'shell', 'getevent', '-p', '-l'], capture_output=True, text=True)
    output = result.stdout.strip()
    value_pattern = re.compile(r"max (\d+),")
    x_abs_mt_max = 0
    y_abs_mt_max = 0

    for line in output.split('\n'):
        if 'ABS_MT_POSITION_X' in line:
            match = re.search(value_pattern, line)
            x_abs_mt_max = int(match.group(1))
        elif 'ABS_MT_POSITION_Y' in line:
            match = re.search(value_pattern, line)
            y_abs_mt_max = int(match.group(1))

    return x_abs_mt_max, y_abs_mt_max


def init_position():
    return [-1, -1, -1]


if __name__ == '__main__':
    screen_width, screen_height = get_screen_resolution()
    x_abs_mt_max, y_abs_mt_mt = get_abs_mt_position()
    x_ratio = screen_width / x_abs_mt_max
    y_ratio = screen_height / y_abs_mt_mt

    adb_command = f"cd C:\\Users\\{configs.computer_user_name}\\AppData\\Local\\Android\\Sdk\\platform-tools && adb devices && adb shell getevent -t -l"
    # os.system(adb_command)

    # 使用subprocess.Popen启动进程
    process = subprocess.Popen(adb_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    # 设置日志记录
    logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

    gesture_list = []
    slot_dict = {}
    current_slot = -1
    position_tobe_added = init_position()
    tracking_id_set = set()

    time_pattern = re.compile(r"\[.*\]")

    gesture_index = 0
    total_str = ""

    # 实时捕获并处理输出
    try:
        for line in process.stdout:
            logging.info(line)

            current_time = None
            if re.match(time_pattern, line):
                current_time = float(re.match(time_pattern, line).group(0)[1:-1])

            if "BTN_TOUCH" in line and "DOWN" in line:
                total_str = ""
            total_str += line

            if "BTN_TOUCH" in line and "DOWN" in line:
                slot_dict = {}
                current_slot = -1
                position_tobe_added = init_position()
                tracking_id_set = set()
            elif "ABS_MT_TRACKING_ID" in line:
                # 存在前面的落单的ABS_MT_POSITION_X没有被添加，就已经切换到下一个slot的情况，因此这里先做一次添加。
                add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)
                position_tobe_added = init_position()

                tracking_id = line.split("ABS_MT_TRACKING_ID")[1]
                if tracking_id.strip() == "ffffffff":
                    continue
                if tracking_id not in tracking_id_set:
                    tracking_id_set.add(tracking_id)
                current_slot = len(tracking_id_set) - 1
                # if slot_dict does not have the slot, add it to the dict
                # 这里有一个假设，即current_slot是从0开始的，且是连续的。
                if current_slot not in slot_dict:
                    slot_dict[current_slot] = []
            elif "ABS_MT_SLOT" in line:
                # 存在前面的落单的ABS_MT_POSITION_X没有被添加，就已经切换到下一个slot的情况，因此这里先做一次添加。
                add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)
                position_tobe_added = init_position()

                slot = int(line.split("ABS_MT_SLOT")[1], 16)
                current_slot = slot
                # if slot_dict does not have the slot, add it to the dict
                if slot not in slot_dict:
                    slot_dict[slot] = []
            elif "ABS_MT_POSITION_X" in line:
                # 前面出现过ABS_MT_POSITION_X，且这个x还没有与y组合成一个点，是单独出现的。则此时我们需要先添加这个单独的x。
                if position_tobe_added[0] != -1:
                    add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)
                    position_tobe_added = init_position()
                hex_value_x = line.split("ABS_MT_POSITION_X")[1]
                position_x = int(hex_value_x, 16) * x_ratio
                position_tobe_added[0] = position_x
                position_tobe_added[2] = current_time
            elif "ABS_MT_POSITION_Y" in line:
                hex_value_y = line.split("ABS_MT_POSITION_Y")[1]
                position_y = int(hex_value_y, 16) * y_ratio
                # 前面已经有了ABS_MT_POSITION_X。
                if position_tobe_added[0] != -1:
                    # 且当前的时间戳与之前的时间戳一致，说明这个x和y是成对的。
                    if current_time == position_tobe_added[2]:
                        position_tobe_added[1] = position_y
                        add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)
                    # 如果时间戳不一致，说明这个x和y是不成对的，是单独出现的。
                    else:
                        add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)
                        position_tobe_added = init_position()
                        position_tobe_added[1] = position_y
                        position_tobe_added[2] = current_time
                        add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)
                # 前面没有ABS_MT_POSITION_X的情况，即当前的ABS_MT_POSITION_Y是单独出现的，则直接添加y。（这里有个前提，就是x与y成对出现的话，一定是先出现x再出现y。）
                else:
                    position_tobe_added[1] = position_y
                    position_tobe_added[2] = current_time
                    add_position_to_slot_dict(position_tobe_added, slot_dict, current_slot)

                position_tobe_added = init_position()
            elif "BTN_TOOL_FINGER" in line and "UP" in line:
                # delete those slots which have no points
                slot_dict = {slot: slot_dict[slot] for slot in slot_dict if len(slot_dict[slot]) > 0}
                file_prefix = f"log/exp_{configs.exp_num}/gesture"
                if not os.path.exists(file_prefix):
                    os.makedirs(file_prefix)

                # write slot_dict into json file
                with open(f"{file_prefix}/gesture_{gesture_index}.json", "w") as json_file:
                    json.dump(
                        {
                            f"gesture_{gesture_index}":
                             {
                                 "events": slot_dict,
                                 "raw_texts": total_str
                             }}, json_file, indent=4, sort_keys=True, ensure_ascii=False, separators=(",", ": "))
                    json_file.write('\n\n')  # 在每个 JSON 对象之后添加双换行符，便于之后读取。

                gesture_index += 1
                gesture_list.append(slot_dict)
                slot_dict = {}

    except KeyboardInterrupt:
        # 如果用户中断程序（Ctrl+C），处理异常并终止进程
        logging.info("User interrupted the program.")
    finally:
        # 等待进程结束
        process.terminate()
        process.wait()

        # 输出进程的返回码
        logging.info("Process exited with return code: %d", process.returncode)