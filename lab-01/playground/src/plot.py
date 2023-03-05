import numpy as np
import pandas as pd
import matplotlib.pyplot as plt



if __name__ == "__main__":
    throughput_single_node_data = pd.read_csv('./throughput-single-node.csv')
    throughput_two_nodes_data = pd.read_csv('./throughput-two-nodes.csv')

    # delay_single_node_data = pd.read_csv('./delay-single-node.csv')
    # delay_two_nodes_data = pd.read_csv('./delay-two-nodes.csv')

    print(throughput_single_node_data)



