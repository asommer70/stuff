
var app = angular.module("testApp", []);

app.directive("addrow", function() {
    return function(scope, element) {
      element.bind("click", function() {

        // Get the table element and the number of rows + 1.
        var table = angular.element(element.parent().parent().parent().parent());
        var rows = angular.element(table.find('tr'));
        var i = rows.length + 1;

        // Copy everything inside the "grid grid-pad" div.
        var new_row = angular.element(rows[1]).clone();

        // Rename each input's name attribute.
        var inputs = new_row.find('input');
        for (var x = 0; x < inputs.length; x++) {
          var input = angular.element(inputs[x]);
          input.attr('name', input.attr('name') + i);
          input.attr('value', '');
        }

        // Append the new row to the dyn table.
        angular.element(table.children()[0]).append(new_row);
      })
    }
});
