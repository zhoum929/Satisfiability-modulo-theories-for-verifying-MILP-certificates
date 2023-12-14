# Satisfiability-modulo-theories-for-verifying-MILP-certificates
## Overview
This is the project for checking **MILP** results (certificates) with **SMT** solver

The `mps_to_smt2.cpp` transforms a **MILP** problem in *mps* format with a bound of optimum value to be verified into **SMT** problem in *smt2* format. The `mps_to_smt2_inf.cpp` transforms a **MILP** problem in *mps* format into **SMT** problem in *smt2* format. The `vipr_to_smt2.cpp` transforms a **MILP** problem certificate in vipr format into **SMT** problem in *smt2* format. The `normalize_num.cpp` modifies the number format of a *smt2* file. Specifically, it changes each negative number in *smt2* file into the minus operator. For instance, `-3` will be changed into `(- 3)`. This is helpful when **SMT** solver does not accept negative numbers as input. The `find_RTP.cpp` prints the problem type and the **RTP** section specified in a *vipr* certificate.

---

The generated *smt2* files can be checked with any **SMT** solvers, such as [cvc5](https://github.com/cvc5/cvc5) and [Z3](https://github.com/Z3Prover/z3)

The authors checked the validity of these codes through the branchmark used in [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments)

If readers want to do test own problems, please refer to [SCIP-extension](https://github.com/leoneifler/exact-SCIP)

## Examples of Usage
Now we give examples of how to run the tests. First, let's use **VIPR2SMT** to verify *vipr* certificate of problem `alu10_5`;
1. Use command `make` to compile programs. (If you prefer to compile with a debugger, use `make DEBUG=1` instead. If you want to use a specific program, you can also compile it solely, with format like `g++ find_RTP.cpp -o find_RTP`) 
2. Download `alu10_5` certificate (*vipr*) file from [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments) to the main folder. The download file is a zipped file, so please extract the *vipr* file inside to the main folder and rename it as `alu10_5.vipr`.
3. Run command `.\vipr_to_smt2`
4. In the terminal, type in `alu10_5.vipr`. Then, the program will generate a file called `alu10_5_vipr.smt2`.
5. Install **cvc5** or other solvers that you want to use. When using **cvc5**, it does not recognize negative numbers, so we need to 
    transfer negative numbers like `-1` into `(- 1)`, which will be done in the next step.
6. Run command `.\normalize_num`.
7. In the terminal, type in `alu10_5_vipr.smt2`. Then, the program will generate a file called `alu10_5_vipr_norm.smt2`.
8. Use your preferred solver to check the generated document. If you are using **cvc5**, for instance, you should run command `cvc5 alu10_5_vipr_norm.smt2`. If it reports `sat`, it means the *vipr* certificate is valid; if it reports `unsat`, it means the *vipr* certificate is invalid.

---

Next we present an example of how to verify the upper/lower bound of the optimum value through **MPS2SMT** transformation. We take the case `flugpl` for example.
1. Download `flugpl.mps` from `/experiements/easy/mps` to the main folder.
2. Download flugpl certificate (*vipr*) file from [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments) to the main folder. The download file is a zipped file, so please extract the vipr file inside to the main folder and rename it as `flugpl.vipr`.
3. We would like to verify the upper/lower bound provided in the **RTP** section of the *vipr* file. Program **find_RTP** can help find this information. Run command `.\find_RTP`, type in `flugpl.vipr`. The result shows that `flugpl` is a minimization problem, so we would like to verify the upper bound of its optimum value. As shown in the result, vipr provides an upper bound of $1201500$ for its optimum value.
4. Run command `.\mps_to_smt2`.
5. Type in `flugpl.mps`, then type in $1201500$. Wait for a while and file `flugpl_mps.smt2` will be generated.
6. If you want to check the generated file with **cvc5**, the negative numbers must be transformed by running `.\normalize_num` and type in `flugpl_mps.smt2`. This will generate file `flugpl_mps_norm.smt2`.
7. Run `cvc5 flugpl_mps_norm.smt2`. This should report `sat`, which means the upper bound of the optimum value provided in *vipr* is valid.

Note that in case of a maximization problem, we should verify lower bound of the optimum value instead. Every other step is the same. The lower and uppor bounds provided in vipr may not be the same--there may be a gap between them.

---

Finally we present an example of how to verify that a problem is infeasible with **MPS2SMT** transformation. We take the case `alu10_5` for example.
1. Download `alu10_5.mps` from `/experiements/hard/mps` to the main folder.
2. Download alu10_5 certificate (*vipr*) file from [vipr-experiements](https://github.com/ambros-gleixner/VIPR/tree/master/experiments) to the main folder. The download file is a zipped file, so please extract the vipr file inside to the main folder and rename it as `alu10_5.vipr`.
3. Run command `.\find_RTP` and type in `alu10_5.vipr`. The result shows that *vipr* states `alu10_5` is infeasible. 
4. Run command `.\mps_to_smt2_inf` and type in `alu10_5.mps`. This will generate file `alu10_5_mps.smt2`.
5. Run command `.\normalize_num` and type in `alu10_5_mps.smt2`. This will generate file `alu10_5_mps_norm.smt2`.
6. Run command `cvc5 alu10_5_mps_norm.smt2`. The result will show `unsat`, which means `alu10_5` is indeed infeasible.

