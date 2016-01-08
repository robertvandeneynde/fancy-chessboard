from numpy import array, zeros, ones
from numpy import cross, dot
from numpy.linalg import norm

def normalized(x):
    return x / norm(x)

# parameters

S = 512
d1 = 0.5
d2 = 0.5
zDir = 'down'

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
        (a,c,i), d1, # (a, c, i) have x = 1
        (b,d,j), d2,
        (f,e,k), 10-d1-d2,
        (g,h), 10-d1
    ],
    [ # y
        (a,b,f,g), d1,
        (c,d,e), d1+d2,
        (j,k), 10-d1-d2,
        (i,h), 10-d1
    ],
    [ # z
        (d,j,e,k), 0 if zDir == 'down' else 1,
        (a,b,f,g,h,i,c), 1 if zDir == 'down' else 0
    ],
]):
    for vtx, v in zip(dim[::2], dim[1::2]):
        for p in vtx:
            p[d] = v
        
print(M)

from PIL import Image

im = Image.new('RGB', (S,S), (255,255,255))
pix = im.load()

from itertools import product, chain, starmap, count

def iterchain(iterables):
    ''' itertools.chain(*tuple(iterables)) '''
    for it in iterables:
        for x in it:
            yield x

def decirange(a,b):
    return range(int(a * S // 10), int(b * S // 10))

# define some ranges, product(range, range) are the points of a rect !
R0, R1, R2, R3, R4, R5 = R = list(starmap(decirange, [
    (0,d1), (d1,d1+d2), (d1+d2,10-d1-d2), (10-d1-d2,10-d1), (10-d1,10), (0,10)
]))

print(R)

normals = nDown, nRight, nUp, nLeft = list(map(normalized, starmap(cross, 
    [(b-f, b-e), (e-g, e-k), (j-k, j-i), (c-d, c-j)]
)))

print(normals)

def normalToColor(N):
    return tuple(int((x + 1) * 255 // 2) for x in N)

saveCounter = count(1)

def save(final=False):
    if final:
        im.save('normal-map&d1={}&d2={}&zDir={}.png'.format(d1,d2,zDir))
        im.save('normal-map.png')
    else:
        im.save('normal-map-{}.png'.format(next(saveCounter)))
    
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



