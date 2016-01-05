from numpy import array, zeros, ones
from numpy import cross, dot
from numpy.linalg import norm

def normalized(x):
    return x / norm(x)

'''
# We define some 3D points
9 i * * * * h
8 * j * * k *
  * * * * * *
  * * * * * *
2 c d * * e *
1 a b * * f g
  1 2     8 9
'''
a,b,c,d, e,f,g,h, i,j,k = M = zeros((11,3))

for d, dim in enumerate([
    [ # x
        (a,c,i), 1, # (a, c, i) have x = 1
        (b,d,j), 2,
        (f,e,k), 8,
        (g,h), 9
    ],
    [ # y
        (a,b,f,g), 1,
        (c,d,e), 2,
        (j,k), 8,
        (i,h), 9
    ],
    [ # z
        (d,j,e,k), 0,
        (a,b,f,g,h,i,c), 1
    ],
]):
    for vtx, v in zip(dim[::2], dim[1::2]):
        for p in vtx:
            p[d] = v
        
print(M)

from PIL import Image

S = 512
im = Image.new('RGB', (S,S), (255,255,255))
pix = im.load()

from itertools import product, chain, starmap, count

def iterchain(iterables):
    ''' itertools.chain(*tuple(iterables)) '''
    for it in iterables:
        for x in it:
            yield x

def decirange(a,b):
    return range(a * S // 10, b * S // 10)

# define some ranges, product(range, range) are the points of a rect !
R0, R1, R2, R3, R4, R5 = R = list(starmap(decirange, [
    (0,1), (1,2), (2,8), (8,9), (9,10), (0,10)
]))

print(R)

normals = nDown, nRight, nUp, nLeft = list(map(normalized, starmap(cross, 
    [(b-f, b-e), (e-g, e-k), (j-k, j-i), (c-d, c-j)]
)))

print(normals)

def normalToColor(N):
    return tuple(int((x + 1) * 255 // 2) for x in N)

c = count(0)

def save(final=False):
    im.save('normal-map-{}.png'.format(next(c)) if not final else
            'normal-map.png')
    
for x,y in iterchain(starmap(product, [
    (R0, R5), (R4, R5), (R5, R0), (R5, R4), (R2, R2)]
)):
    pix[x,y] = normalToColor((0,0,1))
    
save()

for n, (A, B) in zip(
    [nDown, nRight, nUp, nLeft], 
    [(R1, R2), (R3, R2), (R2, R3), (R2, R1)]
):
    for x,y in product(A,B):
        pix[x,y] = normalToColor(n)

save()

for f, (A, B), (na, nb) in zip(
    [lambda x,y : x < y] * 2 + [lambda x,y: y < -x + S] * 2,
    [(R1,R1), (R3, R3), (R3, R1), (R1, R3)],
    [(nDown, nLeft), (nUp, nRight), (nLeft, nRight), (nDown, nUp)],
):
    for x,y in product(A,B):
        pix[x,y] = normalToColor(na if f(x,y) else nb)

save(final=True)



