import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

if __name__ == "__main__":
    t_sn = pd.read_csv('./throughput-single-node-all.csv')
    t_tn = pd.read_csv('./throughput-two-nodes-all.csv')

    d_sn = pd.read_csv('./delay-single-node-all.csv')
    d_tn = pd.read_csv('./delay-two-nodes-all.csv')

    print(t_sn)
    print(t_tn)
    print(d_sn)
    print(d_tn)

    unit_factor = 1_000_000 / 2 ** 20
    t_sn_std_x = t_sn[t_sn['type'] == 'std']['throughput'] * unit_factor
    t_tn_std_x = t_tn[t_tn['type'] == 'std']['throughput'] * unit_factor
    t_sn_buff_x = t_sn[t_sn['type'] == 'buff']['throughput'] * unit_factor
    t_tn_buff_x = t_tn[t_tn['type'] == 'buff']['throughput'] * unit_factor
    t_y = t_tn[t_tn['type'] == 'std']['msgsize'] / 1024
    t_sn_y = t_sn[t_sn['type'] == 'std']['msgsize'] / 1024

    fig, ax = plt.subplots(figsize=(12, 7))
    ax.plot(t_sn_y, t_sn_std_x, label='std single node')
    ax.plot(t_sn_y, t_sn_buff_x, label='buff single node')

    ax.set_title('throughput(message_size)')
    ax.set_ylabel('Throughput [Mb / s]')
    ax.set_xlabel('Message size [KB]')
    ax.legend()

    fig, ax = plt.subplots(figsize=(12, 7))
    ax.plot(t_y, t_tn_std_x, label='std two nodes')
    ax.plot(t_y, t_tn_buff_x, label='buff two nodes')

    ax.set_title('throughput(message_size)')
    ax.set_ylabel('Throughput [Mb / s]')
    ax.set_xlabel('Message size [B]')
    ax.legend()

    plt.show()


