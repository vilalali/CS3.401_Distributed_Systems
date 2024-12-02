# Homework_3

## Team 11

Kapil Rajesh Kavitha - 2021101028 **(Questions 4 and 5)**

Vilal Ali - 2024701027 **(Questions 1,2 and 3)**

# 3.1 Distributed K-Nearest Neighbours (16 points)

### Compilation and Execution:
1. **Compile the code**:
    ```bash
    mpic++ -o 1.out 1.cpp
    ```
2. **Run the program**:
    ```bash
    mpirun -np 12 --use-hwthread-cpus --oversubscribe ./1.out ./1.in
    ```
---
# 3.2 Julia Set (16 points)

### Compilation and Execution:
- **Compile with MPI:**

   ```bash
   mpic++ -o 2.out 2.cpp
   ```

- **Run with MPI:**

   For example, to run with 12 processes:

   ```bash
   mpirun -np 12 --use-hwthread-cpus --oversubscribe ./2.out ./2.in
   ```
---
# 3.3 Prefix Sum (16 Points)

### Compilation and Execution:
1. **Compile the code**:
    ```bash
    mpic++ -o 3.out 3.cpp
    ```
2. **Run the program**:
    ```bash
    mpirun -np 12 --use-hwthread-cpus --oversubscribe ./3.out ./3.in
    ```

3. **Input**:
    You can paste or type your input in the terminal when prompted.


---

# 3.4 Matrix Inversion using Row Reduction 

### Compilation and Execution:
1. **Compile the code**:
    ```bash
    mpic++ -o 4.out 4.cpp
    ```
2. **Run the program**:
    ```bash
      mpirun -np 12 --use-hwthread-cpus --oversubscribe ./4.out ./4.in
      ```

3. **Input**:
      You can paste or type your input in the terminal when prompted.

---

# 3.5 Matrix Chain Multiplication

### Compilation and Execution:

1. **Compile the code**:
    ```bash
    mpic++ -o 5.out 5.cpp
    ```
2. **Run the program**:
    ```bash
      mpirun -np 12 --use-hwthread-cpus --oversubscribe ./5.out ./5.in
      ```