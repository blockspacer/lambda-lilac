import "Core" for Vec2, Vec3, Vec4
import "Core" for Texture, Shader, Mesh
import "Core" for GameObject, Transform, Camera, MeshRender, Lod, RigidBody, WaveSource, Collider
import "Core" for Input, Keys, Buttons, Axes
import "Core" for Math, Graphics, GUI, Time, File, Assert
import "Core" for MonoBehaviour
import "Core" for PostProcess, Console, Physics, Debug

import "resources/scripts/wren/input_controller" for InputController

class DebugDraw {
  static circle(center, radius, segments, colour) {
    for (i in 0..segments) {
      var i1 = i % segments
      var i2 = (i + 1) % segments

      var s1 = Math.sin((i1 / segments) * Math.tau)
      var c1 = Math.cos((i1 / segments) * Math.tau)
      var s2 = Math.sin((i2 / segments) * Math.tau)
      var c2 = Math.cos((i2 / segments) * Math.tau)

      Debug.drawLine(center + Vec3.new(s1 * radius, 0.0, c1 * radius), center + Vec3.new(s2 * radius, 0.0, c2 * radius), colour)
    }
  }
  static line(from, to, radius, colour) {
    var diff = (to - from)
    diff.y = 0.0
    var swap = diff.x
    diff.x = -diff.z
    diff.z = swap
    diff = diff.normalized * radius

    Debug.drawLine(from + diff, to + diff, colour)
    Debug.drawLine(from - diff, to - diff, colour)
  }
}


// class Node {
//   construct new(position) {
//     _index       = -1
//     _position    = position
//     _connections = []
//     _drawIndex   = 0
//     _searchValue = -1
//   }

//   addConnection(connection) {
//     if (index == connection.index) {
//       return
//     }

//     for (c in _connections) {
//       if (c.index == connection.index) {
//         return
//       }
//     }

//     _connections.add(connection)
//     connection.addConnection(this)
//     return this
//   }

//   removeConnection(node) {
//     for (i in 0..._connections.count) {
//       if (_connections[i].index == node.index) {
//         _connections.removeAt(i)
//         return
//       }
//     }
//   }

//   removeConnections() {
//     for (c in _connections) {
//       c.removeConnection(this)
//     }
//     _connections.clear()
//   }

//   evaluateRoute(searchValue) {
//     if (searchValue > _searchValue && _searchValue != -1) {
//       return
//     }

//     _searchValue = searchValue
    
//     for (connection in connections) {
//       var distance = (position - connection.position).length
//       connection.evaluateRoute(searchValue + distance)
//     }
//   }

//   getSearchValue { _searchValue }
//   index { _index }
//   index=(index) { _index = index }

//   reset() {
//     _searchValue = -1
//   }

//   connections { _connections }
//   position    { _position }
// }


// class NodeMap {
//   construct new() {
//     _nodes = []
//   }
  
//   // Modification
//   nodes { _nodes }
//   removeNode(node) {
//     node.removeConnections()
    
//     _nodes.removeAt(node.index)
//     for (j in 0..._nodes.count) {
//       _nodes[j].index = j
//     }
//   }
//   addNode(node) {
//     node.index = _nodes.count
//     _nodes.insert(-1, node)
//   }

//   // Debug
//   draw() {
//     var radius = 0.5
//     var segments = 20
//     var colourLine = Vec4.new(1.0, 0.0, 1.0, 1.0)
//     var colourCircle = Vec4.new(1.0, 0.0, 0.0, 1.0)

//     for (node in _nodes) {
//       for (connection in node.connections) {
//         if (connection.index > node.index) {
//           DebugDraw.line(node.position, connection.position, radius, colourLine)
//         }
//       }
//       DebugDraw.circle(node.position, radius, segments, colourCircle)
//     }
//   }

//   // Pathing
//   findClosest(position) {
//     var closest = -1
//     var closestLength = -1

//     for (node in _nodes) {
//       var length = (position - node.position).lengthSqr
//       if (length < closestLength || closestLength == -1) {
//         closest = node.index
//         closestLength = length
//       }
//     }

//     return closest
//   }
    
//   heuristicCostEstimate(a, b) {
//     return (b.position - a.position).lengthSqr
//   }

//   inArray(node, array) {
//     for (elem in array) {
//       if (node.index == elem.index) {
//         return true
//       }
//     }
//     return false
//   }

//   findPath(startPosition, goalPosition) {
//     var startIndex = findClosest(startPosition)
//     var goalIndex = findClosest(goalPosition)

//     if (startIndex == -1 || goalIndex == -1) {
//       return []
//     }
    
//     var start = _nodes[startIndex]
//     var goal = _nodes[goalIndex]

//     if (start == goal) {
//       return []
//     }

//     var closedSet = []
//     var openSet = [ start ]
//     var cameFrom = {}
//     var gScore = List.filled(_nodes.count, Num.largest)
//     gScore[start.index] = 0
//     var fScore = List.filled(_nodes.count, Num.largest)
//     fScore[start.index] = heuristicCostEstimate(start, goal)

//     while (openSet.count > 0) {
//       var current = null
//       var lowestFScore = -1

//       for (node in openSet) {
//         var nodeFScore = fScore[node.index]
//         if (nodeFScore < lowestFScore || lowestFScore == -1) {
//           lowestFScore = nodeFScore
//           current = node
//         }
//       }

//       if (current == goal) {
//         var totalPath = [ current.position ]
        
//         while (current) {
//           current = cameFrom[current.index]
//           if (current) {
//             totalPath.insert(0, current.position)
//           }
//         }
//         return totalPath
//       }

//       for (i in 0...openSet.count) {
//         if (openSet[i].index == current.index) {
//           openSet.removeAt(i)
//           break
//         }
//       }

//       closedSet.insert(-1, current)

//       for (neighbor in current.connections) {
//         if (!inArray(neighbor, closedSet)) {
//           var tentativeGScore = gScore[current.index] + heuristicCostEstimate(current, neighbor)

//           var valid = true

//           if (!inArray(neighbor, openSet)) {
//               openSet.insert(-1, neighbor)
//           } else {
//             if (tentativeGScore >= gScore[neighbor.index]) {
//               valid = false
//             }
//           }

//           if (valid) {
//             cameFrom[neighbor.index] = current
//             gScore[neighbor.index] = tentativeGScore
//             fScore[neighbor.index] = gScore[neighbor.index] + heuristicCostEstimate(neighbor, goal) 
//           }
//         }
//       }
//     }
//   }
// }

// class NodeEditor {
//   construct new(nodeMap) {
//     _nodeMap = nodeMap

//     _nodePlaceRemove = false
//     _nodeConnect = false
//     _nodeToConnect = null
//   }

//   update(position) {
//     position.x = position.x.round
//     position.y = position.y.round + 0.2
//     position.z = position.z.round

//     DebugDraw.circle(position, 0.5, 20, Vec4.new(0.0, 1.0, 0.0, 1.0))

//     var nodePlaceRemove = InputController.NodePlaceRemove != 0
//     var placeRemove = nodePlaceRemove && !_nodePlaceRemove
//     _nodePlaceRemove = nodePlaceRemove
//     var nodeConnect = InputController.NodeConnect != 0
//     var connect = nodeConnect && !_nodeConnect
//     _nodeConnect = nodeConnect

//     // Place remove
//     if (placeRemove) {
//       var placeNode = true
//       for (node in _nodeMap.nodes) {
//         if (node.position == position) {
//           _nodeMap.removeNode(node)
//           placeNode = false
//           break
//         }
//       }
//       if (placeNode) {
//         _nodeMap.addNode(Node.new(position))
//       }

//       serialize()
//     }

//     if (connect) {
//       var node = null
//       for (n in _nodeMap.nodes) {
//         if (n.position == position) {
//           node = n
//           break
//         }
//       }

//       if (node != null) {
//         if (_nodeToConnect == null) {
//           _nodeToConnect = node
//         } else {
//           if (_nodeToConnect != node) {
//             node.addConnection(_nodeToConnect)
//             _nodeToConnect.addConnection(node)
//           }
//           _nodeToConnect = null
//         }
//       }

//       serialize()
//     }

//     if (_nodeToConnect != null) {
//       DebugDraw.line(_nodeToConnect.position, position, 0.5, Vec4.new(1.0, 1.0, 1.0, 1.0))
//     }
//   }

//   serialize() {
//     var file = File.open("serialized", File.writeOnly)
    
//     var nodes = _nodeMap.nodes
//     for (i in 0...nodes.count) {
//       var node = nodes[i]
//       file.write("%(node.position)|%(node.index)")
//       for (connection in node.connections) {
//         file.write("|%(connection.index)")
//       }

//       if (i != nodes.count - 1) {
//         file.write("\n")
//       }
//     }
    
//     file.close()
//   }

//   deserialize() {
//     var file = File.open("serialized", File.readOnly)

//     var content = file.read()

//     var nodeContent = content.split("\n")
//     var nodes = List.filled(nodeContent.count, null)

//     for (i in 0...nodeContent.count) {
//       if (nodeContent[i].count > 0) {
//         var split = nodeContent[i].split("|")
//         var position = Vec3.fromString(split[0])
//         var nodeIndex = Num.fromString(split[1])
//         nodes[nodeIndex] = Node.new(position)
//       }
//     }

//     for (node in _nodeMap.nodes) {
//       _nodeMap.removeNode(node)
//     }

//     for (i in 0...nodes.count) {
//       if (nodes[i] != null) {
//         _nodeMap.addNode(nodes[i])
//       }
//     }

//     for (i in 0...nodeContent.count) {
//       if (nodeContent[i].count > 0) {
//         var split = nodeContent[i].split("|")
//         var nodeIndex = Num.fromString(split[1])

//         for (j in 2...split.count) {
//           nodes[nodeIndex].addConnection(nodes[Num.fromString(split[j])])
//         }
//       }
//     }

//     file.close()
//   }
// }