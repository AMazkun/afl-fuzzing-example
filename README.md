# american fuzzing lop ++ example

Mathematical function (functional) is tested, which is defined using linear segments and the value on them
A [a,b]

An interval_map<K,V> is a specialized data structure that associates ranges of consecutive keys of type K with values of type V. It's particularly useful when multiple consecutive keys should map to the same value, allowing for a more compact representation than a regular map.

The implementation builds on top of std::map, where each entry (k,v) indicates that the value v is associated with all keys from k (inclusive) to the next key in the map (exclusive). Additionally, a special value m_valBegin represents the value for all keys less than the first key in the map.

For example, given an interval_map<int,char> with:

    m_valBegin = 'A'
    m_map = {(1,'B'), (3,'A')}

The mapping would be:

    All keys less than 1 → 'A'
    Keys 1 and 2 → 'B'
    All keys 3 and greater → 'A'

### Install AFL++

[American Fuzzy Lop ++](https://aflplus.plus/)

For Macos

    ```bash
    brew install afl++
    ```

**Compilation and Fuzzing:**

1.  **Copy and save the code:** Save the C++ code as `add_ints.cpp`.

2.  **Compile for Fuzzing (with AFL++ instrumentation):**

    ```bash
    CXX=afl-clang++ afl-clang++ add_ints.cpp -o add_ints
    ```

3.  **Create an input directory and inital input and seeds:**
    * This creates an initial input file with two integers (1 and 2) in little-endian byte order for mac M1

    ```bash
    mkdir input

    # Create a binary inital input or some se'eds files with hexdump
    echo -ne "\x00\x00\x00\x01\x00\x00\x00\x02" > input/initial_input

    # Create a binary seed file with hexdump
    # standart
    printf '\x2a\x00\x00\x00\x0d\x00\x00\x00\x41' > input/seed1

    # Zero values
    printf '\x00\x00\x00\x00\x00\x00\x00\x00\x58' > input/seed2

    # Negative values
    printf '\xfb\xff\xff\xff\x0a\x00\x00\x00\x5a' > input/seed3

    # Large values
    printf '\x0f\x27\x00\x00\x01\xd9\xff\xff\x2b' > input/seed4
    ```
    
# 4. Compile with CMPLOG
    AFL_LLVM_CMPLOG=1 afl-clang-fast++ -stdlib=libc++ add_ints.cpp -o add_ints

# 5. Config the system before test start
    sudo afl-system-config

# Run with CMPLOG
    **Run AFL++ in persistent mode:**

    ```bash
        afl-fuzz -i input -o output -c ./add_ints ./add_ints
    ```

    * `-i input`: Specifies the input directory.
    * `-o output`: Specifies the output directory (where crashes and new test cases will be stored).
    * `-m none`: Disables memory limits (for simplicity in this example). Use with caution on real targets.
    * `-x none`: Disables dictionary.
    * `--`: Separates AFL++ options from the target program's arguments.
    * `add_ints @@`: Tells AFL++ to run the `add_ints` program, replacing `@@` with the input file.

**Explanation and Crucial Persistent Mode Implementation:**

* **`LLVMFuzzerTestOneInput`:** This is the function that AFL++ calls repeatedly. This is the heart of the persistent mode.
* **Input Handling:**
    * The fuzzer provides the input data as a byte array (`data`) and its size (`size`).
    * The code checks if the input is large enough to contain two integers.
    * `memcpy` is used to copy the integer values from the input buffer.
* **Target Logic:**
    * The code performs a simple addition of the two integers.
    * The if statement within the function is a simple example of a condition that could expose a bug. In real world fuzzing this would be replaced with the code that you want to test.
* **Persistent Execution:** AFL++ handles the persistent execution. The main program is executed once, and `LLVMFuzzerTestOneInput` is called repeatedly with different inputs.
* **`main()` function:** The `main` function is required for the code to link correctly, and to run in normal mode if not being fuzzed.
* **Byte Order:** Note the little-endian byte order when creating the initial input file. This is crucial for correctly interpreting the integer values.

**Important Notes:**

* **Error Handling:** In a real-world scenario, you would need to add more robust error handling and input validation.
* **Complex Targets:** Fuzzing more complex C++ code often requires more intricate setup, including mocking dependencies and handling object states.
* **Sanitizers:** Always use sanitizers (e.g., AddressSanitizer, UndefinedBehaviorSanitizer) when fuzzing to detect memory-related errors. Add `-fsanitize=address,undefined` to the compile command.
* **AFL++ Documentation:** Refer to the official AFL++ documentation for more advanced options and techniques.
* This example minimizes the setup required for persistent mode fuzzing, and it is ready to be run.

