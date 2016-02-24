import QtQuick 2.4

ListModel {
  id: listModel
  property string sortColumnName: ""
  property int order: Qt.AscendingOrder
  property var filters: ({})

  function setFilter(columnName, text) {
    filters[columnName] = text.toLowerCase();
  }

  function checkFilters(item) {
    for(var col in filters) {
      var text = filters[col]

      if(item[col].lastIndexOf(text, 0) !== 0)
        return false;
    }

    return true;
  }

  function buildFromArray(items) {
    clear();

    for(var i = 0; i < items.length; i++) {
      if(checkFilters(items[i]))
        append(items[i]);
    }

    sort();
  }

  function find(columnName, itemName)
  {
    for(var i = 0; i < count; i++) {
      if(get(i)[columnName] == itemName)
        return i;
    }

    return -1;
  }

  function appendFiltered(item) {
    if(checkFilters(item))
      append(item);
  }

  function insertOrdered(item) {
    if(!checkFilters(item))
      return;

    var i;
    for(i = 0; i < count; i++) {
      var a = item[sortColumnName];
      var b = get(i)[sortColumnName];

      if(a != b)
      {
        a = a.toLowerCase();
        b = b.toLowerCase();
      }

      if(order === Qt.AscendingOrder) {
        if(a < b)
          break;
      }
      else if(order === Qt.DescendingOrder) {
        if(a > b)
          break;
      }
    }
    insert(i, item);

    return i;
  }

  function swap(a,b) {
    if (a<b) {
      move(a,b,1);
      move(b-1,a,1);
    }
    else if (a>b) {
      move(b,a,1);
      move(a-1,b,1);
    }
  }

  function partition(begin, end, pivot) {
    var piv=get(pivot)[sortColumnName];

    swap(pivot, end-1);
    var store=begin;
    var ix;

    var a, b;

    for(ix=begin; ix<end-1; ++ix) {
      var item = get(ix)[sortColumnName];

      if(item.toLowerCase() != piv.toLowerCase())
      {
        a = item.toLowerCase();
        b = piv.toLowerCase();
      }
      else
      {
        a = item;
        b = piv;
      }

      if (order === Qt.AscendingOrder) {
        if(a < b) {
          swap(store,ix);
          ++store;
        }
      }
      else if (order === Qt.DescendingOrder) {
        if(a > b) {
          swap(store,ix);
          ++store;
        }
      }
    }
    swap(end-1, store);

    return store;
  }

  function qsort(begin, end) {
    if(end-1>begin) {
      var pivot=begin+Math.floor(Math.random()*(end-begin));

      pivot=partition(begin, end, pivot);

      qsort(begin, pivot);
      qsort(pivot+1, end);
    }
  }

  function sort() {
    qsort(0,count)
  }
}
