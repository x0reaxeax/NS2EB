import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import re

# Function to parse the log file
def parse_log(file_path):
    print(f"Parsing log file: {file_path}..")
    data = {"I": [], "V": []}
    
    with open(file_path, "r") as file:
        for line in file:
            match = re.match(r"(I|V)=0x([0-9A-F]+);T=0x([0-9A-F]+)", line)
            if match:
                msr_type, msr_addr, cycles = match.groups()
                data[msr_type].append((int(msr_addr, 16), int(cycles, 16)))
    
    return data

# Function to plot the graph
def plot_msr_data(data):
    print("Generating MSR graph..")
    plt.figure(figsize=(10, 6))
    
    # Plot invalid MSRs
    if data["I"]:
        i_msr_x, i_msr_y = zip(*data["I"])
        plt.scatter(i_msr_x, i_msr_y, label="Invalid MSRs", alpha=0.7, marker="x")
    
    # Plot valid MSRs
    if data["V"]:
        v_msr_x, v_msr_y = zip(*data["V"])
        plt.scatter(v_msr_x, v_msr_y, label="Valid MSRs", alpha=0.7, marker="o")
    
    plt.xlabel("MSR Address (Hex)")
    plt.ylabel("RDTSC Diff (Cycles)")
    plt.title("MSR Execution Timing")
    plt.legend()
    plt.grid(True)
    
    # Format the X-axis as hexadecimal
    ax = plt.gca()
    ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f"0x{int(x):X}"))
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f'{int(x)}'))

    plt.tight_layout()
    plt.show()

# Main script execution
if __name__ == "__main__":
    log_file = "ns2eb.log"  # Replace with your log file path
    msr_data = parse_log(log_file)
    plot_msr_data(msr_data)
