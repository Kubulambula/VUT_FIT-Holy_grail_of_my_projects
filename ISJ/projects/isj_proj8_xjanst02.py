#!/usr/bin/python3
import collections
my_counter = collections.Counter()


def log_and_count(key=None, counts=None):
    def decorator(func):
        def inner(*args, **kwargs):
            # log
            print("called {0} with {1} and {2}".format(func.__name__, args, kwargs))
            # increment counter
            nonlocal key, counts
            if not counts is None:
                if key is None:
                    key = func.__name__
                counts[key] += 1
            # just call the function
            return func(*args, **kwargs)
        return inner
    return decorator


@log_and_count(key = 'basic functions', counts = my_counter)
def f1(a, b=2):
    return a ** b


@log_and_count(key = 'basic functions', counts = my_counter)
def f2(a, b=3):
    return a ** 2 + b


@log_and_count(counts = my_counter)
def f3(a, b=5):
    return a ** 3 - b


if __name__ == "__main__":
    f1(2)
    f2(2, b=4)
    f1(a=2, b=4)
    f2(4)
    f2(5)
    f3(5)
    f3(5,4)

    print(my_counter)
