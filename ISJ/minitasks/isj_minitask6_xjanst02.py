class Point:
    def __init__(self, x=0, y=0):
        self.x = x
        self.y = y
    
    def __sub__(self, second_point):
        return Point(self.x - second_point.x, self.y - second_point.y)

    def __str__(self):
        return "Point({},{})".format(self.x, self.y)


p0 = Point()
print(p0) # should be: Point(0,0)
p1 = Point(3, 4)
result = p0-p1
print(result) # should be: Point(-3,-4)
