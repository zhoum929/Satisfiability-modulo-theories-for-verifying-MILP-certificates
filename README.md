# Satisfiability-modulo-theories-for-verifying-MILP-certificates
This is the code for checking MILP results (certificates) with SMT solver

The mps_to_smt2.cpp transforms a MILP problem in mps format with a bound of optimum value to be verified into SMT problem in smt2 format. After compile it, run the executable, you can input the filename and the optimum bounds to be verified, and the corresponding smt2 file will be generated.

The mps_to_smt2_inf.cpp transforms a MILP problem in mps format into SMT problem in smt2 format. After compile it, run the executable, you can input the filename, and the corresponding smt2 file will be generated. Note that this transformation doesn't involve upper or lower bounds of the optimum value and can be used only to check feasibility.

The vipr_to_smt2.cpp transforms a MILP problem certificate in vipr format into SMT problem in smt2 format. After compile it, run the executable, you can input the filename, and the corresponding smt2 file will be generated.

The normalize_num.cpp modifies the number format of a smt2 file. Specifically, it changes each negative number in smt2 file into the minus operator. For instance, "-3" will be changed into "(- 3)". This is helpful when SMT solver does not accept negative numbers as input.

The find_RTP.cpp prints the problem type and the RTP section specified in a VIPR certificate.

The generated smt2 files can be checked with any SMT solvers, such as cvc5 https://github.com/Z3Prover/z3 and Z3 https://github.com/Z3Prover/z3 

The authors checked the validity of these codes through the branchmark used in https://github.com/ambros-gleixner/VIPR/tree/master/experiments

