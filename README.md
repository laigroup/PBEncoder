# PB Encoder

Encoding Pseudo-Boolean Constraints into conjunctive normal form with couting safe

A PB formulation to SAT encoding is counting safe iff the number of models in the PB formulation and in the encoded SAT formulation are the same.

We implemented Warners Encoding which posted by J. P. Warners. And Warners encoding is counting safe.


## How to Compile

You will need at least a C++ compiler that supports the C++11 standard. Default compiler is g++. To change this edit the CMakeLists.txt file.

`./INSTALL.sh` 

`./Encoder input_file output_file`


## Format

Input format is the same as PB16 requirements, and there is an example.

```
* #variable= 5 #constraint= 3
****************************************
*
+1 x1 +1 x2 +1 x3 = 2 ;
-1 x3 -1 x4 -1 x5 >= -2 ;
-1 x3 -2 x5 -1 x2 >= -2 ;
```

Output format is the same as DIMACS format in SAT problem.
