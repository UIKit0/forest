#!/usr/bin/env node

var paper = require('./lib/paper-node.js'), fs = require('fs');

function newDrawing(inchWidth, inchHeight) {
    var canvas = new paper.Canvas(inchWidth * 72, inchHeight * 72, 'pdf');
    return require('./fabricator.pjs')(canvas);
}

with (newDrawing(11, 8.5)) {
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
    fs.writeFileSync('panels-drawing.pdf', view.element.toBuffer());
}

with (newDrawing(20*12, 12*12)) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panels = new Group();

    for (var panelNumber = 0; panelNumber < growth.json.panels.length; panelNumber++) {
        console.log('Panel middle ' + panelNumber);

        var panel = new Panel(growth, scale, panelNumber);
        panels.addChild(panel.drawMiddle())
    }

    MakeActualSizePage(panels, scale);
    view.update();
    fs.writeFileSync('panels-middle.pdf', view.element.toBuffer());
}

with (newDrawing(20*12, 12*12)) {
    var growth = new Growth(JSON.parse(fs.readFileSync('../layout/growth.json')));
    var scale = new Scale();
    var panels = new Group();

    for (var panelNumber = 0; panelNumber < growth.json.panels.length; panelNumber++) {
        console.log('Panel front ' + panelNumber);

        var panel = new Panel(growth, scale, panelNumber);
        panels.addChild(panel.drawFront())
    }

    MakeActualSizePage(panels, scale);
    view.update();
    fs.writeFileSync('panels-front.pdf', view.element.toBuffer());
}
