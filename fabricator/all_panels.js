#!/usr/bin/env node

var paper = require('./lib/paper-node.js'), fs = require('fs');
var canvas = new paper.Canvas(792, 612, 'pdf');
var fab = require('./fabricator.pjs')(canvas);

with (fab) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panels = new Group();

    for (var panelNumber = 0; panelNumber < growth.json.panels.length; panelNumber++) {
        console.log('Panel ' + panelNumber);

        var panel = new Panel(growth, scale, panelNumber);
        panels.addChild(panel.draw())
    }

    MakeDiagramPage(panels, scale);
    view.update();
    fs.writeFileSync('panels.pdf', canvas.toBuffer());
}
