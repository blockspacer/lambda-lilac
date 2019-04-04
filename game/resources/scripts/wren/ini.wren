import "Core/Console" for Console
import "Core/File" for File

class Ini {
  construct new(file_name) {
    _items = {}
    _items[""] = {}

    var file = File.open(file_name, File.readOnly)
    var lines = file.read().split("\n")
    file.close()

    var namespace = ""
    var item = ""

    for(line in lines) {
      // Remove all white spaces before and after the line.
      line = line.trim()
      if (line.contains("#")) {
        line = line.split("#")[0]
      }

      // It is a namespace.
      if (line.startsWith("[")) {
        namespace = line.trimStart("[").trimEnd("]")
        
        if (!_items.containsKey(namespace)) {
          _items[namespace] = {}
        }
      }

      // It is an item.
      if (line.contains("=")) {
        var split = line.split("=")
        var lhs = split[0].trim()
        var rhs = split[1].trim()
        
        if (rhs == "true") {
          _items[namespace][lhs] = true
        } else {
          if(rhs == "false") {
            _items[namespace][lhs] = false
          } else {
            _items[namespace][lhs] = Num.fromString(rhs)
          }
        }
      }
    }
  }

  substr(string, offset, count) { 
    var str = string.skip(offset)

    if (count < 0) {
      return str
    }

    str = str.take(str.count - count)
    return str
  }

  [namespace, item] {
    if (!_items.containsKey(namespace)) {
      Console.error("Ini does not have the namespace %(namespace)")
      return
    }
    var ns = _items[namespace]
    if (!ns.containsKey(item)) {
      Console.error("Ini does not have the item %(item) in %(namespace)")
      return
    }

    return ns[item] 
  }
}