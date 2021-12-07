## 三次スプライン曲線のサンプルプログラム
## 参考 : http://marupeke296.com/DXG_No34_UnuniformSprine.html

import numpy as np
import matplotlib.pyplot as plt

def norm( v ):
    return v / np.linalg.norm( v ) if np.linalg.norm( v ) > 0 else v

def get_v( x, y, n ):
    p1 = np.array( [ x[ n + 0 ], y[ n + 0 ], 0 ] )
    p0 = np.array( [ x[ n - 1 ], y[ n - 1 ], 0 ] ) if n >= 0 else p1
    p2 = np.array( [ x[ n + 1 ], y[ n + 1 ], 0 ] ) if n + 2 < len( y ) else p1
    
    if n == 0:
        return norm( p2 - p1 )
    
    if n == len( y ) - 1:
        return norm( p1 - p0 )
    
    a = norm( p0 - p1 )
    b = norm( p2 - p1 )
    
    return norm( b - a )

H = np.matrix( [
    [  2, -2,  1,  1 ],
    [ -3,  3, -2, -1 ],
    [  0,  0,  1,  0 ],
    [  1,  0,  0,  0 ],
] )

y = [ 70, 40, 100, 10 ]
x = [ 0, 100, 200, 300 ]

x2 = []
y2 = []

ts = []

for n in range( len( y ) - 1 ):
    p0 = np.array( [ x[ n + 0 ], y[ n + 0 ], 0 ] )
    p1 = np.array( [ x[ n + 1 ], y[ n + 1 ], 0 ] )
    p2 = np.array( [ x[ n + 2 ], y[ n + 2 ], 0 ] ) if n + 2 < len( y ) else p1
    
    v0 = get_v( x, y, n + 0 )
    v1 = get_v( x, y, n + 1 )
    
    zero = np.array( [ 0, 0, 0 ] )
    
    G = np.matrix(
        [
            p0,
            p1,
            get_v( x, y, n ) * 100, #p1 - p0, # norm( p1 - p0 ),
            get_v( x, y, n + 1 ) * 100, # p2 - p1, # np.array( [ 1, -1, 0 ] ) * 50, # norm( p2 - p1 ), # norm( p1 - p0 )
        ]
    )
    
    for m in range( x[ n ], x[ n + 1 ] ):
        a = ( m - x[ n ] ) / ( x[ n + 1 ] - x[ n ] )
        # print( m, a )
        
        t = np.array( [ a ** 3, a ** 2, a, 1 ] )
        
        v = t @ H @ G
        
        # print( v.shape )
        
        print( m, v[ 0, 0 ], v[ 0, 1 ] )
        
        ts.append( a )
        
        y2.append( v[ 0, 1 ] )
        x2.append( v[ 0, 0 ] )

plt.plot( x, y, alpha=0.5 )
plt.plot( x2, y2 )
# plt.plot( x2, np.array( ts ) * 100 )
plt.show()