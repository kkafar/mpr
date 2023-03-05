import numpy as np
import pandas as pd
import matplotlib.pyplot as plt



if __name__ == "__main__":
    throughput_single_node_data = pd.read_csv('./throughput-single-node-all.csv')
    throughput_two_nodes_data = pd.read_csv('./throughput-two-nodes-all.csv')

    delay_single_node_data = pd.read_csv('./delay-single-node-all.csv')
    delay_two_nodes_data = pd.read_csv('./delay-two-nodes-all.csv')

    print(throughput_single_node_data)
    print(throughput_two_nodes_data)
    print(delay_single_node_data)
    print(delay_two_nodes_data)

    throughput_single_node_std_x = throughput_single_node_data[throughput_single_node_data['type'] == 'std']['throughput']
    throughput_two_nodes_std_x = throughput_two_nodes_data[throughput_two_nodes_data['type'] == 'std']['throughput']
    throughput_single_node_buff_x = throughput_single_node_data[throughput_single_node_data['type'] == 'buff']['throughput']
    throughput_two_nodes_buff_x = throughput_two_nodes_data[throughput_two_nodes_data['type'] == 'buff']['throughput']

    throughput_single_node_std_y = throughput_single_node_data[throughput_single_node_data['type'] == 'std']['msgsize']

    # fig, ax = plt.figure(figsize=(12,7))
    plt.plot(
        throughput_single_node_std_y,
        throughput_single_node_data[throughput_single_node_data['type'] == 'std']['throughput'],
        label='std single node')
    plt.plot(
        throughput_single_node_std_y,
        throughput_single_node_data[throughput_single_node_data['type'] == 'buff']['throughput'],
        label='buff single node')
    # ax.legend()
    # plt.plot(
    #          throughput_single_node_std_y,
    #     throughput_two_nodes_data[throughput_two_nodes_data['type'] == 'std']['throughput'],
    #          label='std two nodes')
    # plt.plot(
    #          throughput_single_node_std_y,
    #     throughput_two_nodes_data[throughput_two_nodes_data['type'] == 'buff']['throughput'],
    #          label='buff two nodes')

    plt.legend()
    plt.show()


