# Vectors, functions, and S2's 1-based indexing.
x <- c(2,4,6,8)
print(mean(x))
print(x[x > 4])
center <- function(x) x - mean(x)
print(center(x))
