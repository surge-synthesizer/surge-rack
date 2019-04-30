//
//  SurgeVCV.js
//  surgeVCV Port
//
//  Created by Dave Palmer on 4/28/19.
//  Copyright © 2019 Decoded Enterprises LLC. All rights reserved.
//
//



//// APIs you can use in your code:
//
// Available methods for drawing into <canvas> elements:
//    SurgeVCV.drawSurgeVCVGUI(canvas, number, targetFrame*, resizing*)
//    SurgeVCV.drawKnobControl(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawPatchPoint(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawControlTextField(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawInputPatch(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawOutputPatch(canvas, cornerRadius, targetFrame*, resizing*)
//
// NOTE: 'canvas' parameter can be either a <canvas> element object, or the id of a <canvas> element in your document.
//
// NOTE: Parameters marked with the '*' symbol are optional
//
// NOTE: Possible arguments for 'resizing' parameter in drawing methods are:
//   'aspectfit': The content is proportionally resized to fit into the target rectangle.
//   'aspectfill': The content is proportionally resized to completely fill the target rectangle.
//   'stretch': The content is stretched to match the entire target rectangle.
//   'center': The content is centered in the target rectangle, but it is NOT resized.
//
// Available Colors:
//    SurgeVCV.surgeBlue
//    SurgeVCV.surgeWhite
//    SurgeVCV.surgeOrange
//    SurgeVCV.color2
//    SurgeVCV.color4
//    SurgeVCV.surgeOrange2
//    SurgeVCV.surgeOrange3
//    SurgeVCV.gradient2Color
//    SurgeVCV.gradient2Color3
//    SurgeVCV.gradient2Color5
//    SurgeVCV.color
//    SurgeVCV.color5
//    SurgeVCV.color6
//    SurgeVCV.fillColor
//    SurgeVCV.color7
//    SurgeVCV.color9
//
// Available Gradients:
//    SurgeVCV.controlDisplay
//
// Available Shadows:
//    SurgeVCV.shadow
//    SurgeVCV.shadow2
//
// Available Utilities:
//    SurgeVCV.clearCanvas(canvas)
//    SurgeVCV.makeRect(x, y, width, height)


//// Create StyleKit Object
var SurgeVCV = {};
(function() {

    //// Drawing Methods

    function drawSurgeVCVGUI(canvas, number, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgevcvPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 450, 380), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 450, resizedFrame.h / 380);
        var resizedShadowScale = Math.min(resizedFrame.w / 450, resizedFrame.h / 380);


        //// Rectangle 75 Drawing
        var rectangle75CornerRadius = 2;
        var rectangle75Rect = makeRect(1.5, 1, 447.5, 324);
        var rectangle75InnerRect = insetRect(rectangle75Rect, rectangle75CornerRadius, rectangle75CornerRadius);
        context.beginPath();
        context.arc(rectangle75InnerRect.x, rectangle75InnerRect.y, rectangle75CornerRadius, Math.PI, 1.5*Math.PI);
        context.arc(rectangle75InnerRect.x + rectangle75InnerRect.w, rectangle75InnerRect.y, rectangle75CornerRadius, 1.5*Math.PI, 2*Math.PI);
        context.lineTo(rectangle75Rect.x+rectangle75Rect.w, rectangle75Rect.y+rectangle75Rect.h);
        context.lineTo(rectangle75Rect.x, rectangle75Rect.y + rectangle75Rect.h);
        context.closePath();
        context.fillStyle = SurgeVCV.color7;
        context.fill();


        //// Rectangle 3 Drawing
        context.beginPath();
        context.rect(0.5, 323.5, 450, 56);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// knobControl1 Drawing
        context.save();
        context.translate(34.72, 102.28);

        var knobControl1Rect = makeRect(0, -57, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl1Rect.x, knobControl1Rect.y, knobControl1Rect.w, knobControl1Rect.h);
        context.clip();
        context.translate(knobControl1Rect.x, knobControl1Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl1Rect.w, knobControl1Rect.h), 'stretch');
        context.restore();

        context.restore();


        //// patchPoint1 Drawing
        context.save();
        context.translate(5, 91);

        var patchPoint1Rect = makeRect(0, -33, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint1Rect.x, patchPoint1Rect.y, patchPoint1Rect.w, patchPoint1Rect.h);
        context.clip();
        context.translate(patchPoint1Rect.x, patchPoint1Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint1Rect.w, patchPoint1Rect.h), 'stretch');
        context.restore();

        context.restore();


        //// Group 34
        context.save();
        context.translate(106.5, 90.5);
        context.scale(1.13, 1);



        //// Group 35
        //// Rectangle 13 Drawing
        roundedRect(context, 0, -33, 86, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(43, -33, 43, 0));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 36
        //// outputText 23 Drawing
        var outputText23Rect = makeRect(4.5, -27.5, 33, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText23TotalHeight = 11 * 1.3;
        context.fillText('Pan', outputText23Rect.x, outputText23Rect.y + 10 + outputText23Rect.h / 2 - outputText23TotalHeight / 2);


        //// outputText 24 Drawing
        var outputText24Rect = makeRect(5.5, -13.5, 72, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText24TotalHeight = 11 * 1.3;
        context.fillText('Left - 100%', outputText24Rect.x, outputText24Rect.y + 10 + outputText24Rect.h / 2 - outputText24TotalHeight / 2);







        context.restore();



        //// knobControl2 Drawing
        var knobControl2Rect = makeRect(34.72, 90.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl2Rect.x, knobControl2Rect.y, knobControl2Rect.w, knobControl2Rect.h);
        context.clip();
        context.translate(knobControl2Rect.x, knobControl2Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl2Rect.w, knobControl2Rect.h), 'stretch');
        context.restore();


        //// patchPoint2 Drawing
        var patchPoint2Rect = makeRect(5, 103, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint2Rect.x, patchPoint2Rect.y, patchPoint2Rect.w, patchPoint2Rect.h);
        context.clip();
        context.translate(patchPoint2Rect.x, patchPoint2Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint2Rect.w, patchPoint2Rect.h), 'stretch');
        context.restore();


        //// Group 3
        //// Group 2
        //// Rectangle 4 Drawing
        roundedRect(context, 106.5, 102.5, 98, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(155.5, 102.5, 155.5, 135.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group
        //// outputText 2 Drawing
        var outputText2Rect = makeRect(112, 108, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText2TotalHeight = 11 * 1.3;
        context.fillText('Delay Time', outputText2Rect.x, outputText2Rect.y + 10 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);


        //// outputText 7 Drawing
        var outputText7Rect = makeRect(113, 122, 84, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText7TotalHeight = 11 * 1.3;
        context.fillText('Left 1.500 / 4th', outputText7Rect.x, outputText7Rect.y + 10 + outputText7Rect.h / 2 - outputText7TotalHeight / 2);








        //// knobControl3 Drawing
        var knobControl3Rect = makeRect(34.72, 135.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl3Rect.x, knobControl3Rect.y, knobControl3Rect.w, knobControl3Rect.h);
        context.clip();
        context.translate(knobControl3Rect.x, knobControl3Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl3Rect.w, knobControl3Rect.h), 'stretch');
        context.restore();


        //// patchPoint3 Drawing
        var patchPoint3Rect = makeRect(5, 148, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint3Rect.x, patchPoint3Rect.y, patchPoint3Rect.w, patchPoint3Rect.h);
        context.clip();
        context.translate(patchPoint3Rect.x, patchPoint3Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint3Rect.w, patchPoint3Rect.h), 'stretch');
        context.restore();


        //// Group 4
        //// Group 5
        //// Rectangle 5 Drawing
        roundedRect(context, 106.5, 147.5, 98, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(155.5, 147.5, 155.5, 180.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 6
        //// outputText 8 Drawing
        var outputText8Rect = makeRect(112, 153, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText8TotalHeight = 11 * 1.3;
        context.fillText('Delay Time', outputText8Rect.x, outputText8Rect.y + 10 + outputText8Rect.h / 2 - outputText8TotalHeight / 2);


        //// outputText 9 Drawing
        var outputText9Rect = makeRect(113, 166, 96, 13);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText9TotalHeight = 11 * 1.3;
        context.fillText('Right 1.500 / 4th', outputText9Rect.x, outputText9Rect.y + 10 + outputText9Rect.h / 2 - outputText9TotalHeight / 2);








        //// knobControl4 Drawing
        var knobControl4Rect = makeRect(35, 180, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl4Rect.x, knobControl4Rect.y, knobControl4Rect.w, knobControl4Rect.h);
        context.clip();
        context.translate(knobControl4Rect.x, knobControl4Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl4Rect.w, knobControl4Rect.h), 'stretch');
        context.restore();


        //// patchPoint4 Drawing
        var patchPoint4Rect = makeRect(5, 193, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint4Rect.x, patchPoint4Rect.y, patchPoint4Rect.w, patchPoint4Rect.h);
        context.clip();
        context.translate(patchPoint4Rect.x, patchPoint4Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint4Rect.w, patchPoint4Rect.h), 'stretch');
        context.restore();


        //// Group 13
        context.save();
        context.translate(106.5, 192.5);
        context.scale(1.13, 1);



        //// Group 14
        //// Rectangle Drawing
        roundedRect(context, 0, 0, 86, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(43, -0, 43, 33));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 15
        //// outputText Drawing
        var outputTextRect = makeRect(4.5, 5.5, 75.33, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputTextTotalHeight = 11 * 1.3;
        context.fillText('Feedback', outputTextRect.x, outputTextRect.y + 10 + outputTextRect.h / 2 - outputTextTotalHeight / 2);


        //// outputText 13 Drawing
        var outputText13Rect = makeRect(5.5, 19.5, 77, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText13TotalHeight = 11 * 1.3;
        context.fillText('- Inf dB', outputText13Rect.x, outputText13Rect.y + 10 + outputText13Rect.h / 2 - outputText13TotalHeight / 2);







        context.restore();



        //// knobControl5 Drawing
        var knobControl5Rect = makeRect(34.72, 225.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl5Rect.x, knobControl5Rect.y, knobControl5Rect.w, knobControl5Rect.h);
        context.clip();
        context.translate(knobControl5Rect.x, knobControl5Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl5Rect.w, knobControl5Rect.h), 'stretch');
        context.restore();


        //// patchPoint5 Drawing
        var patchPoint5Rect = makeRect(5, 238, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint5Rect.x, patchPoint5Rect.y, patchPoint5Rect.w, patchPoint5Rect.h);
        context.clip();
        context.translate(patchPoint5Rect.x, patchPoint5Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint5Rect.w, patchPoint5Rect.h), 'stretch');
        context.restore();


        //// Group 7
        //// Group 8
        //// Rectangle 6 Drawing
        roundedRect(context, 106.5, 237.5, 98, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(155.5, 237.5, 155.5, 270.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 9
        //// outputText 3 Drawing
        var outputText3Rect = makeRect(112, 243, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText3TotalHeight = 11 * 1.3;
        context.fillText('Crossfeed', outputText3Rect.x, outputText3Rect.y + 10 + outputText3Rect.h / 2 - outputText3TotalHeight / 2);


        //// outputText 10 Drawing
        var outputText10Rect = makeRect(113, 257, 84, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText10TotalHeight = 11 * 1.3;
        context.fillText('- Inf dB', outputText10Rect.x, outputText10Rect.y + 10 + outputText10Rect.h / 2 - outputText10TotalHeight / 2);








        //// knobControl6 Drawing
        var knobControl6Rect = makeRect(34.72, 270.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl6Rect.x, knobControl6Rect.y, knobControl6Rect.w, knobControl6Rect.h);
        context.clip();
        context.translate(knobControl6Rect.x, knobControl6Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl6Rect.w, knobControl6Rect.h), 'stretch');
        context.restore();


        //// patchPoint6 Drawing
        var patchPoint6Rect = makeRect(5, 283, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint6Rect.x, patchPoint6Rect.y, patchPoint6Rect.w, patchPoint6Rect.h);
        context.clip();
        context.translate(patchPoint6Rect.x, patchPoint6Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint6Rect.w, patchPoint6Rect.h), 'stretch');
        context.restore();


        //// Group 10
        //// Group 11
        //// Rectangle 7 Drawing
        roundedRect(context, 106.5, 282.5, 98, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(155.5, 282.5, 155.5, 315.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 12
        //// outputText 11 Drawing
        var outputText11Rect = makeRect(112, 288, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText11TotalHeight = 11 * 1.3;
        context.fillText('Low Cut EQ', outputText11Rect.x, outputText11Rect.y + 10 + outputText11Rect.h / 2 - outputText11TotalHeight / 2);


        //// outputText 12 Drawing
        var outputText12Rect = makeRect(113, 301, 96, 13);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText12TotalHeight = 11 * 1.3;
        context.fillText('188 Hz', outputText12Rect.x, outputText12Rect.y + 10 + outputText12Rect.h / 2 - outputText12TotalHeight / 2);








        //// Symbol 15 Drawing
        context.save();
        context.translate(5, 374);

        var symbol15Rect = makeRect(0, -44, 65, 44);
        context.save();
        context.beginPath();
        context.rect(symbol15Rect.x, symbol15Rect.y, symbol15Rect.w, symbol15Rect.h);
        context.clip();
        context.translate(symbol15Rect.x, symbol15Rect.y);

        SurgeVCV.drawInputPatch(canvas, 4, makeRect(0, 0, symbol15Rect.w, symbol15Rect.h), 'stretch');
        context.restore();

        context.restore();


        //// Symbol 16 Drawing
        context.save();
        context.translate(380, 374);

        var symbol16Rect = makeRect(0, -43, 65, 43);
        context.save();
        context.beginPath();
        context.rect(symbol16Rect.x, symbol16Rect.y, symbol16Rect.w, symbol16Rect.h);
        context.clip();
        context.translate(symbol16Rect.x, symbol16Rect.y);

        SurgeVCV.drawOutputPatch(canvas, 4, makeRect(0, 0, symbol16Rect.w, symbol16Rect.h), 'stretch');
        context.restore();

        context.restore();


        //// knobControl7 Drawing
        var knobControl7Rect = makeRect(276, 45, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl7Rect.x, knobControl7Rect.y, knobControl7Rect.w, knobControl7Rect.h);
        context.clip();
        context.translate(knobControl7Rect.x, knobControl7Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl7Rect.w, knobControl7Rect.h), 'stretch');
        context.restore();


        //// patchPoint7 Drawing
        var patchPoint7Rect = makeRect(246, 58, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint7Rect.x, patchPoint7Rect.y, patchPoint7Rect.w, patchPoint7Rect.h);
        context.clip();
        context.translate(patchPoint7Rect.x, patchPoint7Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint7Rect.w, patchPoint7Rect.h), 'stretch');
        context.restore();


        //// Group 31
        context.save();
        context.translate(347.5, 57.5);
        context.scale(1.13, 1);



        //// Group 32
        //// Rectangle 12 Drawing
        roundedRect(context, 0, 0, 86, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(43, -0, 43, 33));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 33
        //// outputText 21 Drawing
        var outputText21Rect = makeRect(4.5, 5.5, 78, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText21TotalHeight = 11 * 1.3;
        context.fillText('High Cut EQ', outputText21Rect.x, outputText21Rect.y + 10 + outputText21Rect.h / 2 - outputText21TotalHeight / 2);


        //// outputText 22 Drawing
        var outputText22Rect = makeRect(5.5, 19.5, 54, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText22TotalHeight = 11 * 1.3;
        context.fillText('7211 Hz', outputText22Rect.x, outputText22Rect.y + 10 + outputText22Rect.h / 2 - outputText22TotalHeight / 2);







        context.restore();



        //// knobControl8 Drawing
        var knobControl8Rect = makeRect(275.72, 90.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl8Rect.x, knobControl8Rect.y, knobControl8Rect.w, knobControl8Rect.h);
        context.clip();
        context.translate(knobControl8Rect.x, knobControl8Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl8Rect.w, knobControl8Rect.h), 'stretch');
        context.restore();


        //// patchPoint8 Drawing
        var patchPoint8Rect = makeRect(246, 103, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint8Rect.x, patchPoint8Rect.y, patchPoint8Rect.w, patchPoint8Rect.h);
        context.clip();
        context.translate(patchPoint8Rect.x, patchPoint8Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint8Rect.w, patchPoint8Rect.h), 'stretch');
        context.restore();


        //// Group 16
        //// Group 17
        //// Rectangle 2 Drawing
        roundedRect(context, 347.5, 102.5, 97, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(396, 102.5, 396, 135.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 18
        //// outputText 4 Drawing
        var outputText4Rect = makeRect(353, 108, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText4TotalHeight = 11 * 1.3;
        context.fillText('Modulation Rate', outputText4Rect.x, outputText4Rect.y + 10 + outputText4Rect.h / 2 - outputText4TotalHeight / 2);


        //// outputText 5 Drawing
        var outputText5Rect = makeRect(354, 122, 84, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText5TotalHeight = 11 * 1.3;
        context.fillText('0.448 Hz', outputText5Rect.x, outputText5Rect.y + 10 + outputText5Rect.h / 2 - outputText5TotalHeight / 2);








        //// knobControl9 Drawing
        var knobControl9Rect = makeRect(275.72, 135.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl9Rect.x, knobControl9Rect.y, knobControl9Rect.w, knobControl9Rect.h);
        context.clip();
        context.translate(knobControl9Rect.x, knobControl9Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl9Rect.w, knobControl9Rect.h), 'stretch');
        context.restore();


        //// patchPoint9 Drawing
        var patchPoint9Rect = makeRect(246, 148, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint9Rect.x, patchPoint9Rect.y, patchPoint9Rect.w, patchPoint9Rect.h);
        context.clip();
        context.translate(patchPoint9Rect.x, patchPoint9Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint9Rect.w, patchPoint9Rect.h), 'stretch');
        context.restore();


        //// Group 19
        //// Group 20
        //// Rectangle 8 Drawing
        roundedRect(context, 347.5, 147.5, 97, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(396, 147.5, 396, 180.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 21
        //// outputText 6 Drawing
        var outputText6Rect = makeRect(353, 153, 89, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText6TotalHeight = 11 * 1.3;
        context.fillText('Modulation Depth', outputText6Rect.x, outputText6Rect.y + 10 + outputText6Rect.h / 2 - outputText6TotalHeight / 2);


        //// outputText 14 Drawing
        var outputText14Rect = makeRect(354, 166, 96, 13);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText14TotalHeight = 11 * 1.3;
        context.fillText('0.0 Cents', outputText14Rect.x, outputText14Rect.y + 10 + outputText14Rect.h / 2 - outputText14TotalHeight / 2);








        //// knobControl10 Drawing
        var knobControl10Rect = makeRect(275.72, 180.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl10Rect.x, knobControl10Rect.y, knobControl10Rect.w, knobControl10Rect.h);
        context.clip();
        context.translate(knobControl10Rect.x, knobControl10Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl10Rect.w, knobControl10Rect.h), 'stretch');
        context.restore();


        //// patchPoint10 Drawing
        var patchPoint10Rect = makeRect(246, 193, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint10Rect.x, patchPoint10Rect.y, patchPoint10Rect.w, patchPoint10Rect.h);
        context.clip();
        context.translate(patchPoint10Rect.x, patchPoint10Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint10Rect.w, patchPoint10Rect.h), 'stretch');
        context.restore();


        //// Group 22
        context.save();
        context.translate(347.5, 192.5);
        context.scale(1.13, 1);



        //// Group 23
        //// Rectangle 9 Drawing
        roundedRect(context, 0, 0, 86, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(43, -0, 43, 33));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 24
        //// outputText 15 Drawing
        var outputText15Rect = makeRect(4.5, 5.5, 75.33, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText15TotalHeight = 11 * 1.3;
        context.fillText('Mix', outputText15Rect.x, outputText15Rect.y + 10 + outputText15Rect.h / 2 - outputText15TotalHeight / 2);


        //// outputText 16 Drawing
        var outputText16Rect = makeRect(5.5, 19.5, 77, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText16TotalHeight = 11 * 1.3;
        context.fillText('0%', outputText16Rect.x, outputText16Rect.y + 10 + outputText16Rect.h / 2 - outputText16TotalHeight / 2);







        context.restore();



        //// knobControl11 Drawing
        var knobControl11Rect = makeRect(275.72, 225.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl11Rect.x, knobControl11Rect.y, knobControl11Rect.w, knobControl11Rect.h);
        context.clip();
        context.translate(knobControl11Rect.x, knobControl11Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl11Rect.w, knobControl11Rect.h), 'stretch');
        context.restore();


        //// patchPoint11 Drawing
        var patchPoint11Rect = makeRect(246, 238, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint11Rect.x, patchPoint11Rect.y, patchPoint11Rect.w, patchPoint11Rect.h);
        context.clip();
        context.translate(patchPoint11Rect.x, patchPoint11Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint11Rect.w, patchPoint11Rect.h), 'stretch');
        context.restore();


        //// Group 25
        //// Group 26
        //// Rectangle 10 Drawing
        roundedRect(context, 347.5, 237.5, 97, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(396, 237.5, 396, 270.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 27
        //// outputText 17 Drawing
        var outputText17Rect = makeRect(353, 243, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText17TotalHeight = 11 * 1.3;
        context.fillText('Mix Width', outputText17Rect.x, outputText17Rect.y + 10 + outputText17Rect.h / 2 - outputText17TotalHeight / 2);


        //// outputText 18 Drawing
        var outputText18Rect = makeRect(354, 257, 84, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText18TotalHeight = 11 * 1.3;
        context.fillText('.25 dB', outputText18Rect.x, outputText18Rect.y + 10 + outputText18Rect.h / 2 - outputText18TotalHeight / 2);








        //// knobControl12 Drawing
        var knobControl12Rect = makeRect(275.72, 270.28, 57, 57);
        context.save();
        context.beginPath();
        context.rect(knobControl12Rect.x, knobControl12Rect.y, knobControl12Rect.w, knobControl12Rect.h);
        context.clip();
        context.translate(knobControl12Rect.x, knobControl12Rect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, knobControl12Rect.w, knobControl12Rect.h), 'stretch');
        context.restore();


        //// patchPoint12 Drawing
        var patchPoint12Rect = makeRect(246, 283, 33, 33);
        context.save();
        context.beginPath();
        context.rect(patchPoint12Rect.x, patchPoint12Rect.y, patchPoint12Rect.w, patchPoint12Rect.h);
        context.clip();
        context.translate(patchPoint12Rect.x, patchPoint12Rect.y);

        SurgeVCV.drawPatchPoint(canvas, 8, makeRect(0, 0, patchPoint12Rect.w, patchPoint12Rect.h), 'stretch');
        context.restore();


        //// Group 28
        //// Group 29
        //// Rectangle 11 Drawing
        roundedRect(context, 347.5, 282.5, 97, 33, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(396, 282.5, 396, 315.5));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 30
        //// outputText 19 Drawing
        var outputText19Rect = makeRect(353, 288, 85, 9);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText19TotalHeight = 11 * 1.3;
        context.fillText('- - -', outputText19Rect.x, outputText19Rect.y + 10 + outputText19Rect.h / 2 - outputText19TotalHeight / 2);


        //// outputText 20 Drawing
        var outputText20Rect = makeRect(354, 301, 96, 13);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText20TotalHeight = 11 * 1.3;
        context.fillText('- - -', outputText20Rect.x, outputText20Rect.y + 10 + outputText20Rect.h / 2 - outputText20TotalHeight / 2);








        //// Group 37
        //// Rectangle 14 Drawing
        var rectangle14Rect = makeRect(86, 56, 17, 36);
        roundedRect(context, rectangle14Rect.x, rectangle14Rect.y, rectangle14Rect.w, rectangle14Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle14Rect.x, rectangle14Rect.y, rectangle14Rect.x, rectangle14Rect.y + rectangle14Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 15 Drawing
        context.beginPath();
        context.rect(88, 58, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 16 Drawing
        context.beginPath();
        context.rect(88, 61, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 17 Drawing
        context.beginPath();
        context.rect(88, 64, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 18 Drawing
        context.beginPath();
        context.rect(88, 67, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 38
        //// Rectangle 19 Drawing
        var rectangle19Rect = makeRect(86, 101, 17, 36);
        roundedRect(context, rectangle19Rect.x, rectangle19Rect.y, rectangle19Rect.w, rectangle19Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle19Rect.x, rectangle19Rect.y, rectangle19Rect.x, rectangle19Rect.y + rectangle19Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 20 Drawing
        context.beginPath();
        context.rect(88, 103, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 21 Drawing
        context.beginPath();
        context.rect(88, 106, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 22 Drawing
        context.beginPath();
        context.rect(88, 109, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 23 Drawing
        context.beginPath();
        context.rect(88, 112, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 39
        //// Rectangle 24 Drawing
        var rectangle24Rect = makeRect(86, 146, 17, 36);
        roundedRect(context, rectangle24Rect.x, rectangle24Rect.y, rectangle24Rect.w, rectangle24Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle24Rect.x, rectangle24Rect.y, rectangle24Rect.x, rectangle24Rect.y + rectangle24Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 25 Drawing
        context.beginPath();
        context.rect(88, 148, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 26 Drawing
        context.beginPath();
        context.rect(88, 151, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 27 Drawing
        context.beginPath();
        context.rect(88, 154, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 28 Drawing
        context.beginPath();
        context.rect(88, 157, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 40
        //// Rectangle 29 Drawing
        var rectangle29Rect = makeRect(86, 190, 17, 36);
        roundedRect(context, rectangle29Rect.x, rectangle29Rect.y, rectangle29Rect.w, rectangle29Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle29Rect.x, rectangle29Rect.y, rectangle29Rect.x, rectangle29Rect.y + rectangle29Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 30 Drawing
        context.beginPath();
        context.rect(88, 192, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 31 Drawing
        context.beginPath();
        context.rect(88, 195, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 32 Drawing
        context.beginPath();
        context.rect(88, 198, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 33 Drawing
        context.beginPath();
        context.rect(88, 201, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 41
        //// Rectangle 34 Drawing
        var rectangle34Rect = makeRect(86, 235, 17, 36);
        roundedRect(context, rectangle34Rect.x, rectangle34Rect.y, rectangle34Rect.w, rectangle34Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle34Rect.x, rectangle34Rect.y, rectangle34Rect.x, rectangle34Rect.y + rectangle34Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 35 Drawing
        context.beginPath();
        context.rect(88, 237, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 36 Drawing
        context.beginPath();
        context.rect(88, 240, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 37 Drawing
        context.beginPath();
        context.rect(88, 243, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 38 Drawing
        context.beginPath();
        context.rect(88, 246, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 42
        //// Rectangle 39 Drawing
        var rectangle39Rect = makeRect(86, 280, 17, 36);
        roundedRect(context, rectangle39Rect.x, rectangle39Rect.y, rectangle39Rect.w, rectangle39Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle39Rect.x, rectangle39Rect.y, rectangle39Rect.x, rectangle39Rect.y + rectangle39Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 40 Drawing
        context.beginPath();
        context.rect(88, 282, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 41 Drawing
        context.beginPath();
        context.rect(88, 285, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 42 Drawing
        context.beginPath();
        context.rect(88, 288, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 43 Drawing
        context.beginPath();
        context.rect(88, 291, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 43
        //// Rectangle 44 Drawing
        var rectangle44Rect = makeRect(327, 56, 17, 36);
        roundedRect(context, rectangle44Rect.x, rectangle44Rect.y, rectangle44Rect.w, rectangle44Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle44Rect.x, rectangle44Rect.y, rectangle44Rect.x, rectangle44Rect.y + rectangle44Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 45 Drawing
        context.beginPath();
        context.rect(329, 58, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 46 Drawing
        context.beginPath();
        context.rect(329, 61, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 47 Drawing
        context.beginPath();
        context.rect(329, 64, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 48 Drawing
        context.beginPath();
        context.rect(329, 67, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 44
        //// Rectangle 49 Drawing
        var rectangle49Rect = makeRect(327, 101, 17, 36);
        roundedRect(context, rectangle49Rect.x, rectangle49Rect.y, rectangle49Rect.w, rectangle49Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle49Rect.x, rectangle49Rect.y, rectangle49Rect.x, rectangle49Rect.y + rectangle49Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 50 Drawing
        context.beginPath();
        context.rect(329, 103, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 51 Drawing
        context.beginPath();
        context.rect(329, 106, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 52 Drawing
        context.beginPath();
        context.rect(329, 109, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 53 Drawing
        context.beginPath();
        context.rect(329, 112, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 45
        //// Rectangle 54 Drawing
        var rectangle54Rect = makeRect(327, 146, 17, 36);
        roundedRect(context, rectangle54Rect.x, rectangle54Rect.y, rectangle54Rect.w, rectangle54Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle54Rect.x, rectangle54Rect.y, rectangle54Rect.x, rectangle54Rect.y + rectangle54Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 55 Drawing
        context.beginPath();
        context.rect(329, 148, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 56 Drawing
        context.beginPath();
        context.rect(329, 151, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 57 Drawing
        context.beginPath();
        context.rect(329, 154, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 58 Drawing
        context.beginPath();
        context.rect(329, 157, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 46
        //// Rectangle 59 Drawing
        var rectangle59Rect = makeRect(327, 190, 17, 36);
        roundedRect(context, rectangle59Rect.x, rectangle59Rect.y, rectangle59Rect.w, rectangle59Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle59Rect.x, rectangle59Rect.y, rectangle59Rect.x, rectangle59Rect.y + rectangle59Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 60 Drawing
        context.beginPath();
        context.rect(329, 192, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 61 Drawing
        context.beginPath();
        context.rect(329, 195, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 62 Drawing
        context.beginPath();
        context.rect(329, 198, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 63 Drawing
        context.beginPath();
        context.rect(329, 201, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 47
        //// Rectangle 64 Drawing
        var rectangle64Rect = makeRect(327, 235, 17, 36);
        roundedRect(context, rectangle64Rect.x, rectangle64Rect.y, rectangle64Rect.w, rectangle64Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle64Rect.x, rectangle64Rect.y, rectangle64Rect.x, rectangle64Rect.y + rectangle64Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 65 Drawing
        context.beginPath();
        context.rect(329, 237, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 66 Drawing
        context.beginPath();
        context.rect(329, 240, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 67 Drawing
        context.beginPath();
        context.rect(329, 243, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 68 Drawing
        context.beginPath();
        context.rect(329, 246, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Group 48
        //// Rectangle 69 Drawing
        var rectangle69Rect = makeRect(327, 280, 17, 36);
        roundedRect(context, rectangle69Rect.x, rectangle69Rect.y, rectangle69Rect.w, rectangle69Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle69Rect.x, rectangle69Rect.y, rectangle69Rect.x, rectangle69Rect.y + rectangle69Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 70 Drawing
        context.beginPath();
        context.rect(329, 282, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 71 Drawing
        context.beginPath();
        context.rect(329, 285, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 72 Drawing
        context.beginPath();
        context.rect(329, 288, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 73 Drawing
        context.beginPath();
        context.rect(329, 291, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// Symbols
        //// bmp00158
        //// SurgeClassicLogo_White
        //// Group 54
        //// Group- 56
        //// Group 57
        //// Path Drawing
        context.beginPath();
        context.moveTo(140.6, 345.57);
        context.bezierCurveTo(139.31, 344.67, 137.99, 344.21, 136.65, 344.21);
        context.bezierCurveTo(133.95, 344.21, 132.61, 345.19, 132.61, 347.13);
        context.bezierCurveTo(132.61, 347.95, 133.32, 348.93, 134.75, 350.07);
        context.lineTo(137.28, 352.12);
        context.bezierCurveTo(138.07, 352.76, 138.61, 353.22, 138.91, 353.52);
        context.bezierCurveTo(139.21, 353.81, 139.53, 354.22, 139.87, 354.74);
        context.bezierCurveTo(140.2, 355.26, 140.45, 355.79, 140.62, 356.32);
        context.bezierCurveTo(140.79, 356.86, 140.87, 357.4, 140.87, 357.96);
        context.bezierCurveTo(140.87, 360.1, 140.03, 361.77, 138.35, 362.98);
        context.bezierCurveTo(136.67, 364.19, 134.48, 364.79, 131.8, 364.79);
        context.bezierCurveTo(129.6, 364.79, 127.52, 364.33, 125.57, 363.43);
        context.lineTo(126.97, 359.32);
        context.bezierCurveTo(128.47, 360.39, 130.21, 360.93, 132.19, 360.93);
        context.bezierCurveTo(133.38, 360.93, 134.37, 360.69, 135.16, 360.2);
        context.bezierCurveTo(135.95, 359.72, 136.34, 359.03, 136.34, 358.13);
        context.bezierCurveTo(136.34, 357.25, 135.63, 356.27, 134.2, 355.2);
        context.lineTo(131.57, 353.2);
        context.bezierCurveTo(130.76, 352.59, 130.22, 352.14, 129.93, 351.86);
        context.bezierCurveTo(129.65, 351.58, 129.35, 351.2, 129.04, 350.7);
        context.bezierCurveTo(128.72, 350.21, 128.49, 349.7, 128.33, 349.17);
        context.bezierCurveTo(128.18, 348.65, 128.1, 348.11, 128.1, 347.56);
        context.bezierCurveTo(128.1, 345.4, 128.85, 343.7, 130.34, 342.44);
        context.bezierCurveTo(131.84, 341.18, 133.8, 340.55, 136.23, 340.55);
        context.bezierCurveTo(137.03, 340.55, 138.06, 340.64, 139.31, 340.82);
        context.bezierCurveTo(140.56, 340.99, 141.44, 341.19, 141.94, 341.41);
        context.lineTo(140.6, 345.57);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();


        //// Path 2 Drawing
        context.beginPath();
        context.moveTo(162.46, 357.17);
        context.bezierCurveTo(161.99, 359.58, 160.82, 361.45, 158.94, 362.79);
        context.bezierCurveTo(157.06, 364.12, 154.74, 364.79, 151.97, 364.79);
        context.bezierCurveTo(149.65, 364.79, 147.77, 364.26, 146.31, 363.2);
        context.bezierCurveTo(144.85, 362.13, 144.12, 360.67, 144.12, 358.8);
        context.bezierCurveTo(144.12, 358.24, 144.18, 357.64, 144.31, 357);
        context.lineTo(147.39, 340.95);
        context.lineTo(151.88, 340.95);
        context.lineTo(148.75, 356.92);
        context.bezierCurveTo(148.69, 357.2, 148.66, 357.51, 148.66, 357.83);
        context.bezierCurveTo(148.66, 358.88, 149.01, 359.7, 149.69, 360.28);
        context.bezierCurveTo(150.37, 360.85, 151.3, 361.14, 152.47, 361.14);
        context.bezierCurveTo(153.97, 361.14, 155.21, 360.75, 156.21, 359.97);
        context.bezierCurveTo(157.2, 359.19, 157.83, 358.1, 158.1, 356.69);
        context.lineTo(161.17, 340.95);
        context.lineTo(165.6, 340.95);
        context.lineTo(162.46, 357.17);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();


        //// Shape Drawing
        context.beginPath();
        context.moveTo(178.94, 354);
        context.lineTo(184.42, 364.39);
        context.lineTo(179.51, 364.39);
        context.lineTo(174.82, 354.69);
        context.bezierCurveTo(174.06, 354.68, 173.12, 354.64, 172.02, 354.57);
        context.lineTo(170.12, 364.39);
        context.lineTo(165.63, 364.39);
        context.lineTo(170.12, 340.95);
        context.bezierCurveTo(173.39, 340.79, 175.53, 340.71, 176.54, 340.71);
        context.bezierCurveTo(179.19, 340.71, 181.28, 341.2, 182.82, 342.17);
        context.bezierCurveTo(184.36, 343.15, 185.13, 344.56, 185.13, 346.41);
        context.bezierCurveTo(185.13, 348.28, 184.56, 349.91, 183.44, 351.29);
        context.bezierCurveTo(182.32, 352.67, 180.82, 353.58, 178.94, 354);
        context.closePath();
        context.moveTo(173.93, 344.55);
        context.lineTo(172.67, 351.11);
        context.bezierCurveTo(173.33, 351.17, 173.92, 351.21, 174.43, 351.21);
        context.bezierCurveTo(178.65, 351.21, 180.77, 349.93, 180.77, 347.37);
        context.bezierCurveTo(180.77, 346.29, 180.36, 345.52, 179.55, 345.07);
        context.bezierCurveTo(178.74, 344.62, 177.4, 344.39, 175.52, 344.39);
        context.bezierCurveTo(175.36, 344.39, 174.83, 344.44, 173.93, 344.55);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();


        //// Path 3 Drawing
        context.beginPath();
        context.moveTo(206.36, 362.26);
        context.bezierCurveTo(203.84, 363.94, 200.75, 364.79, 197.09, 364.79);
        context.bezierCurveTo(194.28, 364.79, 191.99, 363.93, 190.23, 362.2);
        context.bezierCurveTo(188.47, 360.48, 187.59, 358.3, 187.59, 355.65);
        context.bezierCurveTo(187.59, 351.01, 188.93, 347.34, 191.6, 344.62);
        context.bezierCurveTo(194.28, 341.91, 197.93, 340.55, 202.54, 340.55);
        context.bezierCurveTo(205.04, 340.55, 206.91, 341.08, 208.12, 342.15);
        context.lineTo(206.82, 345.62);
        context.bezierCurveTo(205.17, 344.67, 203.54, 344.2, 201.9, 344.2);
        context.bezierCurveTo(198.95, 344.2, 196.58, 345.23, 194.8, 347.29);
        context.bezierCurveTo(193.01, 349.36, 192.11, 351.94, 192.11, 355.04);
        context.bezierCurveTo(192.11, 356.96, 192.67, 358.46, 193.77, 359.53);
        context.bezierCurveTo(194.88, 360.6, 196.4, 361.14, 198.32, 361.14);
        context.bezierCurveTo(199.92, 361.14, 201.31, 360.72, 202.51, 359.88);
        context.lineTo(203.4, 355.24);
        context.lineTo(199.99, 355.24);
        context.lineTo(200.66, 351.75);
        context.lineTo(208.39, 351.75);
        context.lineTo(206.36, 362.26);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();


        //// Path 4 Drawing
        context.beginPath();
        context.moveTo(229.71, 344.6);
        context.lineTo(218.4, 344.6);
        context.lineTo(217.38, 349.99);
        context.lineTo(225.49, 349.99);
        context.lineTo(224.8, 353.48);
        context.lineTo(216.69, 353.48);
        context.lineTo(215.31, 360.74);
        context.lineTo(226.44, 360.74);
        context.lineTo(225.72, 364.39);
        context.lineTo(210.1, 364.39);
        context.lineTo(214.63, 340.95);
        context.lineTo(230.44, 340.95);
        context.lineTo(229.71, 344.6);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();


        //// Shape 2 Drawing
        context.beginPath();
        context.moveTo(299.22, 337.04);
        context.lineTo(293.2, 368.63);
        context.lineTo(235.31, 368.63);
        context.lineTo(241.32, 337.04);
        context.lineTo(299.22, 337.04);
        context.closePath();
        context.moveTo(291.57, 344.95);
        context.lineTo(276.99, 358.64);
        context.lineTo(271.65, 353.53);
        context.lineTo(260.3, 363.88);
        context.lineTo(253.8, 358.44);
        context.lineTo(246.66, 364.04);
        context.lineTo(243.88, 360.55);
        context.lineTo(253.89, 352.68);
        context.lineTo(260.17, 357.94);
        context.lineTo(271.73, 347.41);
        context.lineTo(277.02, 352.48);
        context.lineTo(288.48, 341.72);
        context.bezierCurveTo(289.51, 342.8, 290.54, 343.87, 291.57, 344.95);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();














        //// effectPresetKnob Drawing
        var effectPresetKnobRect = makeRect(89, -1, 57, 57);
        context.save();
        context.beginPath();
        context.rect(effectPresetKnobRect.x, effectPresetKnobRect.y, effectPresetKnobRect.w, effectPresetKnobRect.h);
        context.clip();
        context.translate(effectPresetKnobRect.x, effectPresetKnobRect.y);

        SurgeVCV.drawKnobControl(canvas, makeRect(0, 0, effectPresetKnobRect.w, effectPresetKnobRect.h), 'stretch');
        context.restore();


        //// effectSelectorTextBG Drawing
        context.save();
        context.translate(140.55, 5.5);
        context.scale(1.13, 1);

        roundedRect(context, 0, 0, 172, 44, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(86, -0, 86, 44));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.stroke();

        context.restore();


        //// effectSelectorText
        context.save();
        context.translate(151.81, 16);
        context.scale(1.13, 1);



        //// effectText Drawing
        var effectTextRect = makeRect(-1.76, -4, 122.78, 15);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var effectTextTotalHeight = 13 * 1.3;
        context.fillText('Surge Delay', effectTextRect.x, effectTextRect.y + 12 + effectTextRect.h / 2 - effectTextTotalHeight / 2);


        //// presetText Drawing
        var presetTextRect = makeRect(-1.65, 13, 128.83, 15);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var presetTextTotalHeight = 11 * 1.3;
        context.fillText('Init', presetTextRect.x, presetTextRect.y + 10 + presetTextRect.h / 2 - presetTextTotalHeight / 2);



        context.restore();
        
        context.restore();

    }

    function drawKnobControl(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgevcvPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 57, 57), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 57, resizedFrame.h / 57);
        var resizedShadowScale = Math.min(resizedFrame.w / 57, resizedFrame.h / 57);


        //// knob 2
        context.save();
        context.translate(28.28, 28.72);
        context.rotate(-135 * Math.PI / 180);



        //// Oval 42 Drawing
        oval(context, -20, -20, 40, 40);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color2;
        context.fill();
        context.restore();



        //// Oval 43 Drawing
        oval(context, -15, -14.7, 30, 30);
        context.fillStyle = SurgeVCV.surgeOrange2;
        context.fill();


        //// Oval 44 Drawing
        oval(context, -12, -12, 24, 24);
        context.save();
        SurgeVCV.shadow2(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();



        //// Rectangle 11 Drawing
        var rectangle11CornerRadius = 1;
        var rectangle11Rect = makeRect(-1, -18, 2, 18);
        var rectangle11InnerRect = insetRect(rectangle11Rect, rectangle11CornerRadius, rectangle11CornerRadius);
        context.beginPath();
        context.arc(rectangle11InnerRect.x, rectangle11InnerRect.y, rectangle11CornerRadius, Math.PI, 1.5*Math.PI);
        context.arc(rectangle11InnerRect.x + rectangle11InnerRect.w, rectangle11InnerRect.y, rectangle11CornerRadius, 1.5*Math.PI, 2*Math.PI);
        context.lineTo(rectangle11Rect.x+rectangle11Rect.w, rectangle11Rect.y+rectangle11Rect.h);
        context.lineTo(rectangle11Rect.x, rectangle11Rect.y + rectangle11Rect.h);
        context.closePath();
        context.fillStyle = SurgeVCV.color4;
        context.fill();



        context.restore();
        
        context.restore();

    }

    function drawPatchPoint(canvas, cornerRadius, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgevcvPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 33, 33), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 33, resizedFrame.h / 33);
        var resizedShadowScale = Math.min(resizedFrame.w / 33, resizedFrame.h / 33);


        //// Group
        //// Rectangle 4 Drawing
        roundedRect(context, 1, 1, 31, 31, cornerRadius);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.strokeStyle = SurgeVCV.surgeOrange3;
        context.lineWidth = 2;
        context.lineJoin = 'bevel';
        context.stroke();


        //// connection 5
        //// Oval 17 Drawing
        oval(context, 5, 5, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 18 Drawing
        oval(context, 7, 7, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 19 Drawing
        oval(context, 8, 8, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 20 Drawing
        oval(context, 11, 11, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();
        
        context.restore();

    }

    function drawControlTextField(canvas, cornerRadius, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgevcvPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 86, 33), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 86, resizedFrame.h / 33);


        //// Group 2
        //// Rectangle 4 Drawing
        var rectangle4Rect = makeRect(0, 0, 86, 33);
        roundedRect(context, rectangle4Rect.x, rectangle4Rect.y, rectangle4Rect.w, rectangle4Rect.h, cornerRadius);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle4Rect.x, rectangle4Rect.y, rectangle4Rect.x, rectangle4Rect.y + rectangle4Rect.h));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group
        //// outputText 2 Drawing
        var outputText2Rect = makeRect(4.5, 5.5, 33, 9);
        context.fillStyle = SurgeVCV.color5;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText2TotalHeight = 11 * 1.3;
        context.fillText('Input', outputText2Rect.x, outputText2Rect.y + 10 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);


        //// outputText 7 Drawing
        var outputText7Rect = makeRect(5.5, 19.5, 54, 9);
        context.fillStyle = SurgeVCV.color5;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText7TotalHeight = 11 * 1.3;
        context.fillText('Left 100%', outputText7Rect.x, outputText7Rect.y + 10 + outputText7Rect.h / 2 - outputText7TotalHeight / 2);
        
        context.restore();

    }

    function drawInputPatch(canvas, cornerRadius, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgevcvPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 65, 44), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 65, resizedFrame.h / 44);
        var resizedShadowScale = Math.min(resizedFrame.w / 65, resizedFrame.h / 44);


        //// inputConnections
        //// Rectangle Drawing
        roundedRect(context, 0, 10, 65, 34, cornerRadius);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.fill();


        //// connection 2
        //// Oval 5 Drawing
        oval(context, 38, 16, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 6 Drawing
        oval(context, 40, 18, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 7 Drawing
        oval(context, 41, 19, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 8 Drawing
        oval(context, 44, 22, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// connection
        //// Oval Drawing
        oval(context, 5, 16, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 2 Drawing
        oval(context, 7, 18, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 3 Drawing
        oval(context, 8, 19, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 4 Drawing
        oval(context, 11, 22, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// inputText Drawing
        var inputTextRect = makeRect(0, 0, 33, 9);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var inputTextTotalHeight = 9 * 1.3;
        context.fillText('Input', inputTextRect.x, inputTextRect.y + 9 + inputTextRect.h / 2 - inputTextTotalHeight / 2);
        
        context.restore();

    }

    function drawOutputPatch(canvas, cornerRadius, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgevcvPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 65, 43), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 65, resizedFrame.h / 43);
        var resizedShadowScale = Math.min(resizedFrame.w / 65, resizedFrame.h / 43);


        //// Rectangle 2 Drawing
        roundedRect(context, 0, 9, 65, 34, cornerRadius);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.fill();


        //// outputText Drawing
        var outputTextRect = makeRect(0, 0, 33, 9);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputTextTotalHeight = 9 * 1.3;
        context.fillText('Output', outputTextRect.x, outputTextRect.y + 9 + outputTextRect.h / 2 - outputTextTotalHeight / 2);


        //// connection 3
        //// Oval 9 Drawing
        oval(context, 38, 15, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 10 Drawing
        oval(context, 40, 17, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 11 Drawing
        oval(context, 41, 18, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 12 Drawing
        oval(context, 44, 21, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// connection 4
        //// Oval 13 Drawing
        oval(context, 5, 15, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 14 Drawing
        oval(context, 7, 17, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 15 Drawing
        oval(context, 8, 18, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 16 Drawing
        oval(context, 11, 21, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();
        
        context.restore();

    }

    //// Infrastructure

    function clearCanvas(canvas) {
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        canvas.getContext('2d').clearRect(0, 0, canvas.width, canvas.height);
    }

    // Possible arguments for 'resizing' parameter are:
    //   'aspectfit': The content is proportionally resized to fit into the target rectangle.
    //   'aspectfill': The content is proportionally resized to completely fill the target rectangle.
    //   'stretch': The content is stretched to match the entire target rectangle.
    //   'center': The content is centered in the target rectangle, but it is NOT resized.
    function applyResizingBehavior(resizing, rect, targetRect) {
        if (targetRect === undefined || equalRects(rect, targetRect) || equalRects(targetRect, makeRect(0, 0, 0, 0))) {
            return rect;
        }

        var scales = makeSize(0, 0);
        scales.w = Math.abs(targetRect.w / rect.w);
        scales.h = Math.abs(targetRect.h / rect.h);

        switch (resizing) {
            case 'aspectfit': {
                scales.w = Math.min(scales.w, scales.h);
                scales.h = scales.w;
                break;
            }
            case 'aspectfill': {
                scales.w = Math.max(scales.w, scales.h);
                scales.h = scales.w;
                break;
            }
            case 'stretch':
            case undefined:
                break;
            case 'center': {
                scales.w = 1;
                scales.h = 1;
                break;
            }
            default:
                throw 'Unknown resizing behavior "' + resizing + '". Use "aspectfit", "aspectfill", "stretch" or "center" as resizing behavior.';
        }

        var result = makeRect(Math.min(rect.x, rect.x + rect.w), Math.min(rect.y, rect.y + rect.h), Math.abs(rect.w), Math.abs(rect.h));
        result.w *= scales.w;
        result.h *= scales.h;
        result.x = targetRect.x + (targetRect.w - result.w) / 2;
        result.y = targetRect.y + (targetRect.h - result.h) / 2;
        return result;
    }

    function oval(context, x, y, w, h) {
        context.save();
        context.beginPath();
        context.translate(x, y);
        context.scale(w/2, h/2);
        context.arc(1, 1, 1, 0, 2*Math.PI, false);
        context.closePath();
        context.restore();
    }

    function roundedRect(context, x, y, w, h, r) {
        context.beginPath();
        context.arc(x+r, y+r, r, Math.PI, 1.5*Math.PI);
        context.arc(x+w-r, y+r, r, 1.5*Math.PI, 2*Math.PI);
        context.arc(x+w-r, y+h-r, r, 0, 0.5*Math.PI);
        context.arc(x+r, y+h-r, r, 0.5*Math.PI, Math.PI);
        context.closePath();
    }

    function insetRect(rect, insetX, insetY) {
        return {x: rect.x + insetX, y: rect.y + insetY, w: rect.w - 2*insetX, h: rect.h - 2*insetY};
    }

    function makeRect(x, y, w, h) {
        return { x: x, y: y, w: w, h: h };
    }

    function equalRects(r1, r2) {
        return r1.x === r2.x && r1.y === r2.y && r1.w == r2.w && r1.h === r2.h;
    }

    function makeSize(w, h) {
        return { w: w, h: h };
    }

    function initializeCanvas(canvas) {
        if ('surgevcvPixelRatio' in canvas) return canvas;
        // This function should only be called once on each canvas.
        var context = canvas.getContext('2d');

        var devicePixelRatio = window.devicePixelRatio || 1;
        var backingStorePixelRatio = context.webkitBackingStorePixelRatio
            || context.mozBackingStorePixelRatio
            || context.msBackingStorePixelRatio
            || context.oBackingStorePixelRatio
            || context.backingStorePixelRatio
            || 1;

        var pixelRatio = devicePixelRatio / backingStorePixelRatio;

        canvas.style.width = canvas.width + 'px';
        canvas.style.height = canvas.height + 'px';
        canvas.width *= pixelRatio;
        canvas.height *= pixelRatio;
        canvas.surgevcvPixelRatio = pixelRatio;

        context.scale(pixelRatio, pixelRatio);
        return canvas;
    }

    //// Public Interface

    // Colors
    SurgeVCV.surgeBlue = 'rgba(18, 52, 99, 1)';
    SurgeVCV.surgeWhite = 'rgba(255, 255, 255, 1)';
    SurgeVCV.surgeOrange = 'rgba(255, 144, 0, 1)';
    SurgeVCV.color2 = 'rgba(27, 28, 32, 1)';
    SurgeVCV.color4 = 'rgba(255, 255, 255, 1)';
    SurgeVCV.surgeOrange2 = 'rgba(101, 50, 3, 1)';
    SurgeVCV.surgeOrange3 = 'rgba(227, 112, 8, 1)';
    SurgeVCV.gradient2Color = 'rgba(12, 12, 12, 1)';
    SurgeVCV.gradient2Color3 = 'rgba(29, 29, 29, 1)';
    SurgeVCV.gradient2Color5 = 'rgba(23, 23, 23, 1)';
    SurgeVCV.color = 'rgba(75, 81, 93, 1)';
    SurgeVCV.color5 = 'rgba(0, 133, 230, 1)';
    SurgeVCV.color6 = 'rgba(145, 145, 145, 1)';
    SurgeVCV.fillColor = 'rgba(255, 255, 255, 1)';
    SurgeVCV.color7 = 'rgba(205, 206, 212, 1)';
    SurgeVCV.color9 = 'rgba(156, 157, 160, 1)';

    // Gradients
    SurgeVCV.controlDisplay = function (g) {
    g.addColorStop(0, SurgeVCV.gradient2Color);
    g.addColorStop(0.51, 'rgb(0, 0, 0)');
    g.addColorStop(0.75, SurgeVCV.gradient2Color3);
    g.addColorStop(1, SurgeVCV.gradient2Color5);
    return g;
};

    // Shadows
    SurgeVCV.shadow = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 1*pixelRatio;
    context.shadowBlur = 2*pixelRatio;
    context.shadowColor = 'rgba(0, 0, 0, 0.5)';
};

    SurgeVCV.shadow2 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 1*pixelRatio;
    context.shadowBlur = 2*pixelRatio;
    context.shadowColor = 'rgb(0, 0, 0)';
};

    // Drawing Methods
    SurgeVCV.drawSurgeVCVGUI = drawSurgeVCVGUI;
    SurgeVCV.drawKnobControl = drawKnobControl;
    SurgeVCV.drawPatchPoint = drawPatchPoint;
    SurgeVCV.drawControlTextField = drawControlTextField;
    SurgeVCV.drawInputPatch = drawInputPatch;
    SurgeVCV.drawOutputPatch = drawOutputPatch;

    // Utilities
    SurgeVCV.clearCanvas = clearCanvas;
    SurgeVCV.makeRect = makeRect;

})();
