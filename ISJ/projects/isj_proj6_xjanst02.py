#!/usr/bin/python3
from itertools import zip_longest


class Polynomial:
    #
    #
    # I coded __init__() by the specification, tested it with the provided asserts and my own tests,
    # but the test server still refuses to give me the point and I don't know why :(
    #
    #
    def __init__(self, *args, **kwargs):
        # Index coresponds to the order of the coefficient
        # self.coefficients[0] ~ x^0
        # self.coefficients[1] ~ x^1
        # ...
        # self.coefficients[n] ~ x^n
        self.coefficients = [0]

        if args:
            # Set coeficients to the first arg if it is list or all the args
            self.coefficients = args[0] if isinstance(args[0], list) else args
        if kwargs:
            # Filter the kwargs to those that would match the "x%d" regex
            filtered_kwargs = {key: value for key, value in kwargs.items() if key.startswith('x') and key[1:].isdigit()}
            # Check if no valid args were given. If so, give a default value and abort
            if len(filtered_kwargs) == 0:
                self.coefficients = [0]
                return
            # Find the highest coeficient as int
            max_power = int(max([key[1:] for key in filtered_kwargs.keys()]))
            # Fill the self.coefficients list with zeroes
            self.coefficients = [0] * (max_power + 1)
            # Set all the specified coefficients
            for key in filtered_kwargs.keys():
                self.coefficients[int(key[1:])] = filtered_kwargs[key]
        # Filters all the non-int values and replaces them with 0
        self.coefficients = [(coefficient if isinstance(coefficient, int) else 0) for coefficient in self.coefficients]
        # Cut off any trailing 0s (high coefficients with no impact)
        while len(self.coefficients) > 0 and self.coefficients[-1] == 0:
            self.coefficients.pop(-1)
        # Add back a 0 coefficient if all coefficients were 0
        if len(self.coefficients) == 0:
            self.coefficients = [0]
        

    def __str__(self):
        out = ""
        for i in range(len(self.coefficients)-1, -1, -1):
            # Sign of the curent polynominal coefficient
            sign = "+" if self.coefficients[i] >= 0 else "-"
            # Absolute value of the base of the curent polynominal coefficient
            base = abs(self.coefficients[i]) if abs(self.coefficients[i]) != 1 or i == 0 else ""
            # x if x should be printed
            x = "x" if i >= 1 else ""
            # Power of the curent polynominal coefficient
            exponent = (f"^{i}" if i > 1 else "")
            # Format it all together
            out += f"{sign}{base}{x}{exponent}" if self.coefficients[i] or len(self.coefficients) == 1 else ""
        # Strip the leading "+" and pad all the other sign symbols with spaces so that it looks pretty
        return out.lstrip("+").replace("+", " + ").replace("-", " - ")


    def __eq__(self, other):
        # First check if they are the same length
        if len(self.coefficients) != len(other.coefficients):
            return False
        # Then check if all the items match
        for i in range(len(self.coefficients)):
            if self.coefficients[i] != other.coefficients[i]:
                return False
        return True


    def __add__(self, other):
        coefficients = []
        # zip_longest() works like zip(), but uses fillvalue as a fallback if no value is present in one of the shorter lists
        for vals in zip_longest(self.coefficients, other.coefficients, fillvalue=0):
            # append the addition result to coefficients
            coefficients.append(vals[0] + vals[1])
        return Polynomial(coefficients)


    def __pow__(self, power):
        return self if power == 1 else self * pow(self, power-1)
    

    def __mul__(self, other):
        # Coefficients lengths
        n = len(self.coefficients)
        m = len(other.coefficients)
        # Create a list long enough for all the results
        result = [0] * (m + n - 1)
        for i in range(n):
            for ii in range(m):
                # Multiply every element with earch other and add together those, that have the same order (same index in the coefficients list)
                result[i + ii] += self.coefficients[i] * other.coefficients[ii]
        return Polynomial(result)


    def derivative(self):
        return Polynomial([i*c for i, c in enumerate(self.coefficients)][1:])
    

    def at_value(self, value1, value2=None):
        if value2 is None: # If value2 is not set
            result = 0
            for i in range(len(self.coefficients)):
                # Multiply each coefficient with the value raised to the power of the coefficient's order
                result += self.coefficients[i] * (value1 ** i)
            return result
        # If value2 is not None, return the difference
        return self.at_value(value2) - self.at_value(value1)
