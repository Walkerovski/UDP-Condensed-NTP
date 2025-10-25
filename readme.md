# UDP Time Synchronization Client and Server

## Overview

This project involves implementing both a UDP client and server that perform a simplified version of the Network Time Protocol (NTP). The client sends timestamped requests to the server, which responds with its own timestamps. Using these exchanges, the client can compute time offset and round-trip delay.

---

## Protocol

The system uses two message types:

1. **TimeRequest** (Client → Server)
   Contains sequence number and client timestamp.

2. **TimeResponse** (Server → Client)
   Contains sequence number, client timestamp, and server timestamp.

### TimeRequest Format

| Field              | Type    | Description                                             |
| ------------------ | ------- | ------------------------------------------------------- |
| Sequence Number    | 4 bytes | Integer in network byte order identifying request order |
| Version            | 4 bytes | Integer (always `7`)                                    |
| Client Seconds     | 8 bytes | Time in seconds when the client sent the request        |
| Client Nanoseconds | 8 bytes | Time in nanoseconds when the client sent the request    |

### TimeResponse Format

| Field              | Type    | Description                                           |
| ------------------ | ------- | ----------------------------------------------------- |
| Sequence Number    | 4 bytes | Same as in the request                                |
| Version            | 4 bytes | Integer (always `7`)                                  |
| Client Seconds     | 8 bytes | From the original request                             |
| Client Nanoseconds | 8 bytes | From the original request                             |
| Server Seconds     | 8 bytes | Time in seconds when the server sent the response     |
| Server Nanoseconds | 8 bytes | Time in nanoseconds when the server sent the response |

---

## Server Implementation

### Usage

```bash
server -p <port> -d <drop_rate>
```

### Arguments

* `-p <Number>`: Port to bind to and listen on (must be > 1024)
* `-d <Number>`: Optional drop rate (percentage of packets to ignore).

  * Range: 0–100
  * Default: 0 (no packets dropped)

### Example

```bash
server -p 57177 -d 50
```

### Behavior

1. Binds to the specified UDP port and listens for `TimeRequest` packets.
2. Randomly drops packets according to the specified drop rate.
3. For each valid request:

   * Takes a timestamp using `clock_gettime()`.
   * Tracks the highest sequence number received per client.
   * If a request’s sequence number is lower than the highest seen, prints:

     ```
     <ADDR>:<PORT> <SEQ> <MAX>
     ```
   * Sends a `TimeResponse` containing the original client timestamps and the new server timestamp.
4. If no updates occur for a client after two minutes, its record is cleared.

---

## Client Implementation

### Usage

```bash
client -a <address> -p <port> -n <count> -t <timeout>
```

### Arguments

* `-a <String>`: Server IP address (e.g., `128.8.126.63`)
* `-p <Number>`: Server port number
* `-n <Number>`: Number of `TimeRequest` messages to send (≥ 0)
* `-t <Number>`: Timeout in seconds for waiting on responses

  * `0` means wait indefinitely

### Example

```bash
client -a 128.8.126.63 -p 57178 -n 100 -t 5
```

### Behavior

1. Sends `n` sequential `TimeRequest` packets (sequence numbers start from 1).
2. Each packet includes a timestamp from `clock_gettime()`.
3. Waits for corresponding `TimeResponse` messages.
4. For each received response:

   * Takes a new local timestamp `T2`.
   * Extracts client (`T0`) and server (`T1`) timestamps from the message.
   * Computes:

     ```
     θ = ((T1 - T0) + (T1 - T2)) / 2
     δ = T2 - T0
     ```

     where:

     * θ = clock offset
     * δ = round-trip delay

### Output Format

Each line corresponds to one sequence number:

```
<SEQ>: <THETA> <DELTA>
```

If no response was received within timeout:

```
<SEQ>: Dropped
```

#### Example Output

```
1: -0.0555 0.5551
2: Dropped
3: 0.1055 1.1230
4: Dropped
```