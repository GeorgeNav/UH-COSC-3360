#!/usr/bin/env python3
import subprocess
import os
import itertools
import random

walsh_codes = [[-1, 1, -1, 1], [-1, -1, 1, 1], [-1, 1, 1, -1]]

def test(d1, d2, d3, m1, m2, m3):
    return "{0} {1}\n{2} {3}\n{4} {5}\n".format(d1, m1, d2, m2, d3, m3).encode()

def encoder_string(pid, m, d):
    return "Here is the message from child {0}: Value = {1}, Destination = {2}".format(pid, m, d)

def encoder_output(d1, d2, d3, m1, m2, m3):
    s1 = encoder_string(1, m1, d1)
    s2 = encoder_string(2, m2, d2)
    s3 = encoder_string(3, m3, d3)
    return "{0}\n{1}\n{2}\n".format(s1, s2, s3)

def walsh_encode(m, W):
    B = [int(b) for b in bin(m)[2:]]
    B = [0] * (3 - len(B)) + B
    answer = []
    for b in B:
        if b == 0:
            answer += [-w for w in W]
        else:
            answer += W
    return answer

def encode(m1, m2, m3):
    w1, w2, w3 = walsh_codes
    EM1 = walsh_encode(m1, w1)
    EM2 = walsh_encode(m2, w2)
    EM3 = walsh_encode(m3, w3)
    return [EM1[i] + EM2[i] + EM3[i] for i in range(12)]

def decoder_string(pid, m, d):
    return "Child {0}, sending value: {1} to child process {2}".format(pid, m, d)

def decoder_output(d1, d2, d3, m1, m2, m3):
    M = [0, 0, 0]
    m = [m1, m2, m3]
    D = [0, 0, 0]
    answer = ''
    for i, d in enumerate([d1, d2, d3]):
        M[d - 1] = m[i]
        D[d - 1] = i
        answer += decoder_string(i + 1, m[i], d) + "\n"
    answer += "\n"
    for pid in range(1, 4):
        answer += "Child {0}".format(pid) + '\n'
        answer += "Signal: " + " ".join([str(x) for x in encode(m1, m2, m3)]) + "\n"
        answer += "Code: " + " ".join([str(x) for x in walsh_codes[D[pid - 1]]]) + "\n"
        answer += "Received value = {0}".format(M[pid - 1]) + "\n\n"
    return answer[:-1]

def run_server(port=6016):
    command = os.getcwd() + "/server"
    server = subprocess.Popen([command, str(port)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return server

def run_client(d1, d2, d3, m1, m2, m3, port=6016):
    command = os.getcwd() + "/client"
    test_case = test(d1, d2, d3, m1, m2, m3)
    client = subprocess.check_output([command, str(port)], input=test_case)
    return client.decode("utf-8")

def main():
    messages = [(x, y, z) for x in range(8) for y in range(8) for z in range(8)]
    for m1, m2, m3 in messages:
        for d1, d2, d3 in itertools.permutations([1, 2, 3]):
            while True:
                port = random.randint(1000, 2 ** 16)
                server = run_server(port)
                client = run_client(d1, d2, d3, m1, m2, m3, port).replace(" ", "").replace("\n", "")
                server = server.communicate()[0].decode("utf-8").replace(" ", "").replace("\n", "")
                expected_server = encoder_output(d1, d2, d3, m1, m2, m3).replace(" ", "").replace("\n", "")
                expected_client = decoder_output(d1, d2, d3, m1, m2, m3).replace(" ", "").replace("\n", "")
                if server == expected_server and client == expected_client:
                    print(d1, d2, d3, m1, m2, m3, port, "pass")
                    break
                else:
                    i = 0
                    while i < len(server) and i < len(expected_server) and server[i] == expected_server[i]:
                        i += 1
                    if i < len(server) and i < len(expected_server):
                        print("Character {0} in server output is {1}, expected {2}".format(i, server[i], expected_server[i]))
                        print(server)
                        print(expected_server)
                    i = 0
                    while i < len(client) and i < len(expected_client) and client[i] == expected_client[i]:
                        i += 1
                    if i < len(client) and i < len(expected_client):
                        print("Character {0} in client output is {1}, expected {2}".format(i, client[i], expected_client[i]))
                        print(client)
                        print(expected_client)
                    print(d1, d2, d3, m1, m2, m3, port, "fail")
            
if __name__ == "__main__":
    main()
