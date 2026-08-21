def fib(num):
    if num <= 1:
        return num
    x, y = 0, 1
    for _ in range(2, num + 1):
        x, y = y, x + y
    return y

if __name__ == '__main__':
    for k in range(10):
        print(fib(k))
