# 星平面オブジェクトを追加する
import math
doc = MQSystem.getDocument()
obj = MQSystem.newObject()
# 頂点の追加
radius = 1
# 面頂点は左ねじ
vis = []
for i in range(10):
	ang = -2.0 * math.pi * i / 10.0
	z = math.cos(ang)
	x = math.sin(ang)
	rr = radius * (1 if (i & 1) else 0.5)
	index = obj.addVertex(x * rr, 0.0, z * rr)
	vis.append(index)
face = []
face.append(obj.addFace(vis))
# 面への材質の設定
mat = MQSystem.newMaterial()
mat.color.red = 0.5
mat.color.green = 0.0
mat.color.blue = 1.0
mat.specular = 5.0
index = doc.addMaterial(mat)
for f in face:
	obj.face[f].material = index
doc.addObject(obj)
