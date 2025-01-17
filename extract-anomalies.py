import re

def extract_anomalies(input_file, output_file, threshold):
    """
    Extracts anomalies from the log file where clock cycles exceed a threshold.
    
    Args:
        input_file (str): Path to the input log file.
        output_file (str): Path to save the anomalies.
        threshold (int): Clock cycle threshold to classify as an anomaly.
    """
    anomalies = []

    try:
        with open(input_file, 'r') as infile:
            for line in infile:
                # Match log lines with MSR format: "=0x<address>;T=0x<time>"
                match = re.search(r"T=0x([0-9A-Fa-f]+)", line)
                if match:
                    time_cycles = int(match.group(1), 16)  # Convert hex to int
                    if time_cycles > threshold:
                        anomalies.append(line.strip())

        # Write anomalies to the output file
        with open(output_file, 'w') as outfile:
            outfile.write("\n".join(anomalies))
        
        print(f"[+] Extracted {len(anomalies)} anomalies into '{output_file}'.")

    except FileNotFoundError:
        print(f"[-] File '{input_file}' not found!")
    except Exception as e:
        print(f"[-] Error: {e}")

# Example usage
if __name__ == "__main__":
    input_log = "ns2eb.log"
    output_log = "anomalies.log"
    anomaly_threshold = 0x10000  # Adjust threshold as needed

    extract_anomalies(input_log, output_log, anomaly_threshold)