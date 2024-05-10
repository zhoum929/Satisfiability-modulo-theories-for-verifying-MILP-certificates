# Satisfiability-modulo-theories-for-verifying-MILP-certificates
## Overview
This is the project for checking **MILP** results (certificates) with **SMT** solver

## Programs Description
1. `mps_to_smt2 <filename> <optimum bound>` transforms a feasible **MILP** problem in *mps* format with a bound of optimum value to be verified into **SMT** problem in *smt2* format.
2. `mps_to_smt2_inf <filename>` transforms an infeasible **MILP** problem in *mps* format into **SMT** problem in *smt2* format.
3. `vipr_to_smt2 <filename>` transforms a **MILP** problem certificate in vipr format into **SMT** problem in *smt2* format.
4. `normalize_num <filename>` modifies the number format of a *smt2* file. Specifically, it changes each negative number in *smt2* file into the minus operator. For instance, `-3` will be changed into `(- 3)`. This is helpful when **SMT** solver does not accept negative numbers as input. 
5. `find_RTP <filename>` prints the problem type and the **RTP** section specified in a *vipr* certificate.
6. `viprsmtchk <filename> <whether specify blocksize (Y/N)> <blocksize>` verifies a **MILP** problem certificate in vip format, by transforming its lines block by block into **SMT** instances, where blocksize is the maximum number of lines encoded in a **SMT** instance, and check each block with **SMT** solver cvc5.
7. `viprsmtchkpal <filename> <whether specify blocksize (Y/N)> <blocksize>` is the parallelization of `viprsmtchk`.
---

The generated *smt2* files can be checked with any **SMT** solvers, such as [cvc5](https://github.com/cvc5/cvc5) and [Z3](https://github.com/Z3Prover/z3)

The authors checked the validity of these codes through the benchmarks used in [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments)

If readers want to do test own problems, please refer to [SCIP-extension](https://github.com/leoneifler/exact-SCIP)

## Examples of Usage
Now we give examples of how to run the tests. First, let's use **VIPR2SMT** to verify *vipr* certificate of problem `alu10_5`;
1. Use command `make` to compile programs. (If you prefer to compile with a debugger, use `make DEBUG=1` instead. If you want to use a specific program, you can also compile it solely, with format like `g++ find_RTP.cpp -o find_RTP`) 
2. Download `alu10_5` certificate (*vipr*) file from [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments) to the main folder. The download file is a zipped file, so please extract the *vipr* file inside to the main folder and rename it as `alu10_5.vipr`.
3. Run command `./vipr_to_smt2 alu10_5.vipr`. Then, the program will generate a file called `alu10_5_vipr.smt2`.
4. Install **cvc5** or other solvers that you want to use. When using **cvc5**, it does not recognize negative numbers, so we need to 
    transfer negative numbers like `-1` into `(- 1)`, which will be done in the next step.
5. Run command `./normalize_num alu10_5_vipr.smt2`. Then, the program will generate a file called `alu10_5_vipr_norm.smt2`.
6. Use your preferred solver to check the generated document. If you are using **cvc5**, for instance, you should run command `cvc5 alu10_5_vipr_norm.smt2`. If it reports `sat`, it means the *vipr* certificate is valid; if it reports `unsat`, it means the *vipr* certificate is invalid.

Alternatively, we can use `viprsmtchk` to verify *vipr* certificate of problem `alu10_5`. This program uses cvc5 as the default **SMT** solver. If we want to specify the blocksize (maximum number of DER constraints in *vipr* certificate that could be encoded in a single **SMT** instance) to be 50, run command `./viprsmtchk alu10_5.vipr Y 50`. If we don't want to specify the block size, run command `./viprsmtchk alu10_5.vipr N`. Then, it will print `alu10_5.vipr is valid` and report the time used.

If we want more efficiency, we can run the parallelization of `viprsmtchk` with the following steps:
1. Change the 7th line of local_runner.sh to the directory where cvc5 is installed; Change the 8th line to the working directory.
2. Change the 105th line of `remote_execution_manager.cpp` to the working directory; Change line 17-19 to the machines to be used (fill in the hostname and the maximum number of processors in each machine).
3. Run command `g++ -std=c++11 -o viprsmtchkpal viprsmtchkpal.cpp remote_execution_manager.cpp -lpthread` to complie the program.
4. If we want to specify the blocksize to be 50, run command `./viprsmtchk alu10_5.vipr Y 50`. If we don't want to specify the block size, run command `./viprsmtchk alu10_5.vipr N`. Then, it will print `alu10_5.vipr is valid` and report the time used.


---

Next we present an example of how to verify the upper/lower bound of the optimum value through **MPS2SMT** transformation. We take the case `flugpl` for example.
1. Download `flugpl.mps` from `/experiements/easy/mps` to the main folder.
2. Download flugpl certificate (*vipr*) file from [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments) to the main folder. The download file is a zipped file, so please extract the vipr file inside to the main folder and rename it as `flugpl.vipr`.
3. We would like to verify the upper/lower bound provided in the **RTP** section of the *vipr* file. Program **find_RTP** can help find this information. Run command `./find_RTP flugpl.vipr`. The result shows that `flugpl` is a minimization problem, so we would like to verify the upper bound of its optimum value. As shown in the result, vipr provides an upper bound of $1201500$ for its optimum value.
4. Run command `./mps_to_smt2 flugpl.mps 1201500`. Wait for a while and file `flugpl_mps.smt2` will be generated.
5. If you want to check the generated file with **cvc5**, the negative numbers must be transformed by running `./normalize_num flugpl_mps.smt2`. This will generate file `flugpl_mps_norm.smt2`.
6. Run `cvc5 flugpl_mps_norm.smt2`. This should report `sat`, which means the upper bound of the optimum value provided in *vipr* is valid.

Note that in case of a maximization problem, we should verify lower bound of the optimum value instead. Every other step is the same. The lower and uppor bounds provided in vipr may not be the same--there may be a gap between them.

---

Finally we present an example of how to verify that a problem is infeasible with **MPS2SMT** transformation. We take the case `alu10_5` for example.
1. Download `alu10_5.mps` from `/experiements/hard/mps` to the main folder.
2. Download alu10_5 certificate (*vipr*) file from [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments) to the main folder. The download file is a zipped file, so please extract the vipr file inside to the main folder and rename it as `alu10_5.vipr`.
3. Run command `./find_RTP alu10_5.vipr`. The result shows that *vipr* states `alu10_5` is infeasible. 
4. Run command `./mps_to_smt2_inf alu10_5.mps`. This will generate file `alu10_5_mps.smt2`.
5. Run command `./normalize_num alu10_5_mps.smt2`. This will generate file `alu10_5_mps_norm.smt2`.
6. Run command `cvc5 alu10_5_mps_norm.smt2`. The result will show `unsat`, which means `alu10_5` is indeed infeasible.

