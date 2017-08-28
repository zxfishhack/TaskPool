#!/bin/bash
str=$1
str=${str/.cpp/.o}
str=${str/.c/.co}
str=${str//\.\.\//}
str=${str//\//\\/}
sed "s/_f_u_c_k_/$str/" $2
