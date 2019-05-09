//
//  SurgeVCV.js
//  surgeVCV Port
//
//  Created by Dave Palmer on 5/8/19.
//  Copyright © 2019 Decoded Enterprises LLC. All rights reserved.
//




//// APIs you can use in your code:
//
// Available methods for drawing into <canvas> elements:
//    SurgeVCV.drawSurgeVCVGUI(canvas, number, targetFrame*, resizing*)
//    SurgeVCV.drawPatchPoint(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawControlTextField(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawInputPatch(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawOutputPatch(canvas, cornerRadius, targetFrame*, resizing*)
//    SurgeVCV.drawKnobControl2(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawSurgeKnob(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas1(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas2(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas3(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawSurgeKnobRooster(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas5(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas6(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawSurgeKnob_34x34(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawSurgeKnobRooster2(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawADSR(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas4(canvas, number, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas7(canvas, number, targetFrame*, resizing*)
//    SurgeVCV.drawADSR2(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawADSR3(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawCanvas8(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawLFO(canvas, targetFrame*, resizing*)
//    SurgeVCV.drawButtonBank(canvas, targetFrame*, resizing*)
//
// Available methods that return generated images:
//    SurgeVCV.imageOfKnobControl(pixelRatio)
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
//    SurgeVCV.shadow4Color
//    SurgeVCV.shadow5Color
//    SurgeVCV.shadow10Color
//    SurgeVCV.shadow11Color
//    SurgeVCV.shadow12Color
//    SurgeVCV.shadow13Color
//
// Available Gradients:
//    SurgeVCV.controlDisplay
//
// Available Shadows:
//    SurgeVCV.shadow
//    SurgeVCV.shadow2
//    SurgeVCV.shadow3
//    SurgeVCV.shadow4
//    SurgeVCV.shadow5
//    SurgeVCV.shadow6
//    SurgeVCV.shadow7
//    SurgeVCV.shadow8
//    SurgeVCV.shadow9
//    SurgeVCV.shadow10
//    SurgeVCV.shadow11
//    SurgeVCV.shadow12
//    SurgeVCV.shadow13
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
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 450, 380), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 450, resizedFrame.h / 380);
        var resizedShadowScale = Math.min(resizedFrame.w / 450, resizedFrame.h / 380);


        //// Color Declarations
        var gradient19Color = 'rgba(205, 205, 205, 1)';
        var gradient19Color2 = 'rgba(185, 185, 185, 1)';

        //// Gradient Declarations
        function gradient19(g) {
            g.addColorStop(0, gradient19Color2);
            g.addColorStop(1, gradient19Color);
            return g;
        }

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
        context.translate(35, 102);

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
        context.translate(5, 362);

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
        context.translate(380, 362);

        var symbol16Rect = makeRect(0, -43, 65, 43);
        context.save();
        context.beginPath();
        context.rect(symbol16Rect.x, symbol16Rect.y, symbol16Rect.w, symbol16Rect.h);
        context.clip();
        context.translate(symbol16Rect.x, symbol16Rect.y);

        SurgeVCV.drawOutputPatch(canvas, 4, makeRect(0, 0, symbol16Rect.w, symbol16Rect.h), 'stretch');
        context.restore();

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



        //// Symbol Drawing
        var symbolRect = makeRect(274, 47, 57, 57);
        context.save();
        context.beginPath();
        context.rect(symbolRect.x, symbolRect.y, symbolRect.w, symbolRect.h);
        context.clip();
        context.translate(symbolRect.x, symbolRect.y);

        SurgeVCV.drawKnobControl2(canvas, makeRect(0, 0, symbolRect.w, symbolRect.h), 'stretch');
        context.restore();


        //// Symbol 2 Drawing
        var symbol2Rect = makeRect(288, 103, 33, 33);
        context.save();
        context.beginPath();
        context.rect(symbol2Rect.x, symbol2Rect.y, symbol2Rect.w, symbol2Rect.h);
        context.clip();
        context.translate(symbol2Rect.x, symbol2Rect.y);

        SurgeVCV.drawSurgeKnob(canvas, makeRect(0, 0, symbol2Rect.w, symbol2Rect.h), 'stretch');
        context.restore();


        //// Symbol 3 Drawing
        var symbol3Rect = makeRect(277, 138, 50, 50);
        context.save();
        context.beginPath();
        context.rect(symbol3Rect.x, symbol3Rect.y, symbol3Rect.w, symbol3Rect.h);
        context.clip();
        context.translate(symbol3Rect.x, symbol3Rect.y);

        SurgeVCV.drawCanvas1(canvas, makeRect(0, 0, symbol3Rect.w, symbol3Rect.h), 'stretch');
        context.restore();


        //// Symbol 4 Drawing
        var symbol4Rect = makeRect(278, 185, 50, 50);
        context.save();
        context.beginPath();
        context.rect(symbol4Rect.x, symbol4Rect.y, symbol4Rect.w, symbol4Rect.h);
        context.clip();
        context.translate(symbol4Rect.x, symbol4Rect.y);

        SurgeVCV.drawCanvas2(canvas, makeRect(0, 0, symbol4Rect.w, symbol4Rect.h), 'stretch');
        context.restore();


        //// Symbol 5 Drawing
        var symbol5Rect = makeRect(278, 228, 50, 50);
        context.save();
        context.beginPath();
        context.rect(symbol5Rect.x, symbol5Rect.y, symbol5Rect.w, symbol5Rect.h);
        context.clip();
        context.translate(symbol5Rect.x, symbol5Rect.y);

        SurgeVCV.drawCanvas3(canvas, makeRect(0, 0, symbol5Rect.w, symbol5Rect.h), 'stretch');
        context.restore();


        //// Symbol 6 Drawing
        var symbol6Rect = makeRect(278, 271, 50, 50);
        context.save();
        context.beginPath();
        context.rect(symbol6Rect.x, symbol6Rect.y, symbol6Rect.w, symbol6Rect.h);
        context.clip();
        context.translate(symbol6Rect.x, symbol6Rect.y);

        SurgeVCV.drawSurgeKnobRooster(canvas, makeRect(0, 0, symbol6Rect.w, symbol6Rect.h), 'stretch');
        context.restore();


        //// Symbol 7 Drawing
        var symbol7Rect = makeRect(36, 275, 50, 50);
        context.save();
        context.beginPath();
        context.rect(symbol7Rect.x, symbol7Rect.y, symbol7Rect.w, symbol7Rect.h);
        context.clip();
        context.translate(symbol7Rect.x, symbol7Rect.y);

        SurgeVCV.drawCanvas5(canvas, makeRect(0, 0, symbol7Rect.w, symbol7Rect.h), 'stretch');
        context.restore();


        //// Symbol 8 Drawing
        var symbol8Rect = makeRect(36, 230, 50, 50);
        context.save();
        context.beginPath();
        context.rect(symbol8Rect.x, symbol8Rect.y, symbol8Rect.w, symbol8Rect.h);
        context.clip();
        context.translate(symbol8Rect.x, symbol8Rect.y);

        SurgeVCV.drawCanvas6(canvas, makeRect(0, 0, symbol8Rect.w, symbol8Rect.h), 'stretch');
        context.restore();


        //// Group 50
        //// Oval Drawing
        oval(context, 8, 364, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(15, 364, 15, 378));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 49
        context.save();
        context.translate(15, 371);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 74 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 76 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 51
        //// Oval 2 Drawing
        oval(context, 8, 3, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(15, 3, 15, 17));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 52
        context.save();
        context.translate(15, 10);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 77 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 78 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 53
        //// Oval 3 Drawing
        oval(context, 428, 364, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(435, 364, 435, 378));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 55
        context.save();
        context.translate(435, 371);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 79 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 80 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 58
        //// Oval 4 Drawing
        oval(context, 428, 3, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(435, 3, 435, 17));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 59
        context.save();
        context.translate(435, 10);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 81 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 82 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Symbol 9 Drawing
        var symbol9Rect = makeRect(47, 194, 34, 34);
        context.save();
        context.beginPath();
        context.rect(symbol9Rect.x, symbol9Rect.y, symbol9Rect.w, symbol9Rect.h);
        context.clip();
        context.translate(symbol9Rect.x, symbol9Rect.y);

        SurgeVCV.drawSurgeKnobRooster2(canvas, makeRect(0, 0, symbol9Rect.w, symbol9Rect.h), 'stretch');
        context.restore();
        
        context.restore();

    }

    function drawKnobControl(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 50, 50), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 50, resizedFrame.h / 50);
        var resizedShadowScale = Math.min(resizedFrame.w / 50, resizedFrame.h / 50);


        //// knob 2
        context.save();
        context.translate(34.28, 35.72);
        context.rotate(-360 * Math.PI / 180);



        //// Oval 42 Drawing
        oval(context, -28, -29, 36, 36);
        context.fillStyle = SurgeVCV.color2;
        context.fill();


        //// Oval 43 Drawing
        oval(context, -23, -23.7, 26, 26);
        context.fillStyle = SurgeVCV.surgeOrange2;
        context.fill();


        //// Oval 44 Drawing
        oval(context, -20, -21, 20, 20);
        context.save();
        SurgeVCV.shadow2(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();



        //// Rectangle 11 Drawing
        var rectangle11CornerRadius = 0.5;
        var rectangle11Rect = makeRect(-10, -27, 1, 16);
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
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 33, 33), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 33, resizedFrame.h / 33);
        var resizedShadowScale = Math.min(resizedFrame.w / 33, resizedFrame.h / 33);


        //// Group
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
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 86, 33), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 86, resizedFrame.h / 33);


        //// Group 2
        //// Rectangle 4 Drawing
        var rectangle4Rect = makeRect(0, 0, 83, 33);
        roundedRect(context, rectangle4Rect.x, rectangle4Rect.y, rectangle4Rect.w, rectangle4Rect.h, cornerRadius);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle4Rect.x, rectangle4Rect.y, rectangle4Rect.x, rectangle4Rect.y + rectangle4Rect.h));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group
        //// outputText 2 Drawing
        var outputText2Rect = makeRect(4.5, 5.5, 32, 9);
        context.fillStyle = SurgeVCV.color5;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText2TotalHeight = 11 * 1.3;
        context.fillText('Input', outputText2Rect.x, outputText2Rect.y + 10 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);


        //// outputText 7 Drawing
        var outputText7Rect = makeRect(5.5, 19.5, 52, 9);
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
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
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
        var inputTextRect = makeRect(22.5, 10, 33, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
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
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
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
        var outputTextRect = makeRect(18, 9, 33, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
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

    function drawKnobControl2(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 57, 57), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 57, resizedFrame.h / 57);
        var resizedShadowScale = Math.min(resizedFrame.w / 57, resizedFrame.h / 57);


        //// Color Declarations
        var gradientColor = 'rgba(99, 65, 0, 1)';
        var gradientColor2 = 'rgba(255, 176, 0, 1)';
        var color8 = 'rgba(214, 214, 214, 1)';
        var color10 = 'rgba(255, 255, 255, 0.22)';
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color11 = 'rgba(5, 0, 87, 1)';
        var gradient3Color = 'rgba(94, 91, 162, 1)';
        var gradient3Color2 = 'rgba(94, 91, 162, 1)';
        var gradient3Color4 = 'rgba(28, 22, 167, 1)';
        var color12 = 'rgba(255, 255, 255, 0.28)';

        //// Gradient Declarations
        function gradient(g) {
            g.addColorStop(0, gradientColor);
            g.addColorStop(0.45, blendedColor(gradientColor, gradientColor2, 0.5));
            g.addColorStop(1, gradientColor2);
            return g;
        }
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient3(g) {
            g.addColorStop(0, gradient3Color2);
            g.addColorStop(0.49, gradient3Color4);
            g.addColorStop(1, gradient3Color);
            return g;
        }

        //// Oval Drawing
        oval(context, 7, 7, 43, 43);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = gradient(context.createRadialGradient(28.5, 28.5, 12.64, 28.5, 28.5, 35.62));
        context.fill();
        context.restore();



        //// Oval 2 Drawing
        oval(context, 10, 10, 37, 37);
        context.save();
        SurgeVCV.shadow3(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = color8;
        context.fill();
        context.restore();

        context.strokeStyle = color10;
        context.lineWidth = 1.5;
        context.stroke();


        //// Oval 3 Drawing
        oval(context, 11, 11, 35, 35);
        context.fillStyle = gradient2(context.createLinearGradient(28.5, 11, 28.5, 46));
        context.fill();


        //// Oval 4 Drawing
        oval(context, 26, 12, 5, 5);
        context.save();
        SurgeVCV.shadow4(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = gradient3(context.createLinearGradient(28.5, 12, 28.5, 17));
        context.fill();
        context.restore();

        context.strokeStyle = color11;
        context.lineWidth = 1;
        context.stroke();


        //// Oval 5 Drawing
        oval(context, 27.5, 12.5, 2, 2);
        context.fillStyle = color12;
        context.fill();
        
        context.restore();

    }

    function drawSurgeKnob(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 25, 25), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 25, resizedFrame.h / 25);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }

        //// Group
        //// Group 2
        //// Oval 3 Drawing
        oval(context, 0.25, 0.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(12.5, 0.25, 12.5, 24.75));
        context.fill();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(18.84, 4.2);
        context.bezierCurveTo(18.76, 4.38, 18.72, 4.58, 18.72, 4.79);
        context.bezierCurveTo(18.72, 5.62, 19.39, 6.29, 20.22, 6.29);
        context.bezierCurveTo(20.42, 6.29, 20.62, 6.25, 20.8, 6.17);
        context.bezierCurveTo(21.87, 7.57, 22.59, 9.25, 22.83, 11.09);
        context.bezierCurveTo(22.51, 11.11, 22.21, 11.23, 21.97, 11.42);
        context.bezierCurveTo(21.63, 11.69, 21.42, 12.11, 21.42, 12.58);
        context.bezierCurveTo(21.42, 13.37, 22.02, 14.01, 22.79, 14.08);
        context.bezierCurveTo(22.53, 15.7, 21.89, 17.2, 20.96, 18.48);
        context.bezierCurveTo(20.74, 18.35, 20.49, 18.28, 20.22, 18.28);
        context.bezierCurveTo(20.1, 18.28, 19.98, 18.29, 19.87, 18.32);
        context.bezierCurveTo(19.21, 18.47, 18.72, 19.07, 18.72, 19.78);
        context.bezierCurveTo(18.72, 20.07, 18.8, 20.35, 18.95, 20.58);
        context.bezierCurveTo(17.52, 21.69, 15.8, 22.44, 13.91, 22.69);
        context.bezierCurveTo(13.88, 22.03, 13.41, 21.48, 12.78, 21.32);
        context.bezierCurveTo(12.67, 21.29, 12.54, 21.28, 12.42, 21.28);
        context.bezierCurveTo(11.63, 21.28, 10.99, 21.88, 10.92, 22.65);
        context.bezierCurveTo(9.24, 22.38, 7.69, 21.7, 6.38, 20.73);
        context.bezierCurveTo(6.59, 20.47, 6.72, 20.14, 6.72, 19.78);
        context.bezierCurveTo(6.72, 19.25, 6.45, 18.79, 6.04, 18.52);
        context.bezierCurveTo(5.81, 18.37, 5.52, 18.28, 5.22, 18.28);
        context.bezierCurveTo(4.86, 18.28, 4.53, 18.41, 4.27, 18.62);
        context.bezierCurveTo(3.28, 17.29, 2.6, 15.73, 2.34, 14.02);
        context.bezierCurveTo(2.97, 13.84, 3.42, 13.26, 3.42, 12.58);
        context.bezierCurveTo(3.42, 12.05, 3.14, 11.58, 2.72, 11.32);
        context.bezierCurveTo(2.6, 11.24, 2.45, 11.17, 2.3, 11.13);
        context.bezierCurveTo(2.54, 9.23, 3.29, 7.49, 4.42, 6.06);
        context.bezierCurveTo(4.65, 6.2, 4.93, 6.29, 5.22, 6.29);
        context.bezierCurveTo(6.05, 6.29, 6.72, 5.62, 6.72, 4.79);
        context.bezierCurveTo(6.72, 4.52, 6.65, 4.26, 6.52, 4.04);
        context.bezierCurveTo(6.7, 3.91, 6.89, 3.78, 7.08, 3.66);
        context.bezierCurveTo(7.26, 3.55, 7.45, 3.44, 7.64, 3.33);
        context.bezierCurveTo(8.67, 2.78, 9.79, 2.39, 10.98, 2.21);
        context.bezierCurveTo(11.16, 2.83, 11.74, 3.29, 12.42, 3.29);
        context.bezierCurveTo(12.89, 3.29, 13.31, 3.07, 13.58, 2.74);
        context.bezierCurveTo(13.71, 2.57, 13.81, 2.38, 13.87, 2.17);
        context.bezierCurveTo(15.72, 2.4, 17.42, 3.13, 18.84, 4.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 2 Drawing
        oval(context, 4.5, 4.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(12.5, 4.5, 12.5, 20.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle Drawing
        roundedRect(context, 11.4, 1.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        
        context.restore();

    }

    function drawCanvas1(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 50, 50), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 50, resizedFrame.h / 50);
        var resizedShadowScale = Math.min(resizedFrame.w / 50, resizedFrame.h / 50);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color12 = 'rgba(255, 255, 255, 0.28)';
        var color17 = 'rgba(0, 0, 0, 1)';
        var gradient6Color = 'rgba(33, 32, 39, 1)';
        var gradient6Color2 = 'rgba(58, 58, 70, 1)';
        var color18 = 'rgba(85, 85, 85, 1)';
        var gradient7Color = 'rgba(52, 51, 51, 1)';
        var gradient7Color2 = 'rgba(90, 90, 90, 1)';
        var color20 = 'rgba(0, 0, 0, 0.2)';
        var gradient8Color = 'rgba(205, 206, 211, 1)';
        var gradient8Color2 = 'rgba(57, 57, 59, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient6(g) {
            g.addColorStop(0, gradient6Color);
            g.addColorStop(1, gradient6Color2);
            return g;
        }
        function gradient7(g) {
            g.addColorStop(0, gradient7Color);
            g.addColorStop(1, gradient7Color2);
            return g;
        }
        function gradient8(g) {
            g.addColorStop(0, gradient8Color);
            g.addColorStop(1, gradient8Color2);
            return g;
        }

        //// Oval Drawing
        oval(context, 5, 5, 40, 40);
        context.save();
        SurgeVCV.shadow2(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = gradient6(context.createLinearGradient(25, 45, 25, 5));
        context.fill();
        context.restore();

        context.strokeStyle = color17;
        context.lineWidth = 1;
        context.stroke();


        //// Oval 2 Drawing
        oval(context, 10, 10, 30, 30);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = gradient8(context.createLinearGradient(25, 10, 25, 40));
        context.fill();

        ////// Oval 2 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow4(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();

        context.strokeStyle = color18;
        context.lineWidth = 1;
        context.stroke();


        //// Oval 3 Drawing
        oval(context, 20.5, 11.5, 9, 9);
        context.fillStyle = gradient7(context.createLinearGradient(25, 11.5, 25, 20.5));
        context.fill();

        ////// Oval 3 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow9(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.strokeStyle = color20;
        context.lineWidth = 0.5;
        context.stroke();


        //// Oval 4 Drawing
        oval(context, 22, 13, 6, 6);
        context.fillStyle = gradient2(context.createLinearGradient(25, 13, 25, 19));
        context.fill();


        //// Oval 5 Drawing
        oval(context, 23, 13, 4, 4);
        context.fillStyle = color12;
        context.fill();
        
        context.restore();

    }

    function drawCanvas2(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 50, 50), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 50, resizedFrame.h / 50);
        var resizedShadowScale = Math.min(resizedFrame.w / 50, resizedFrame.h / 50);


        //// Color Declarations
        var gradient7Color = 'rgba(52, 51, 51, 1)';
        var gradient7Color2 = 'rgba(90, 90, 90, 1)';
        var gradient9Color = 'rgba(255, 255, 255, 1)';
        var gradient9Color2 = 'rgba(154, 154, 154, 1)';
        var gradient10Color = 'rgba(78, 78, 78, 1)';
        var gradient10Color2 = 'rgba(125, 125, 125, 1)';

        //// Gradient Declarations
        function gradient7(g) {
            g.addColorStop(0, gradient7Color);
            g.addColorStop(1, gradient7Color2);
            return g;
        }
        function gradient9(g) {
            g.addColorStop(0, gradient9Color);
            g.addColorStop(0.51, gradient9Color2);
            g.addColorStop(1, 'rgb(255, 255, 255)');
            return g;
        }
        function gradient10(g) {
            g.addColorStop(0, gradient10Color2);
            g.addColorStop(1, gradient10Color);
            return g;
        }

        //// Oval 2 Drawing
        oval(context, 3.5, 3.5, 43, 43);
        context.save();
        SurgeVCV.shadow2(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = gradient7(context.createLinearGradient(25, 46.5, 25, 3.5));
        context.fill();

        ////// Oval 2 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow6(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();



        //// Bezier Drawing
        context.beginPath();
        context.moveTo(21.86, 6.89);
        context.bezierCurveTo(22.72, 7.58, 23.81, 8, 25, 8);
        context.bezierCurveTo(26.86, 8, 28.49, 6.98, 29.35, 5.47);
        context.bezierCurveTo(31.53, 5.96, 33.59, 6.8, 35.44, 7.94);
        context.lineTo(35.39, 8.07);
        context.bezierCurveTo(35.14, 8.66, 35, 9.31, 35, 10);
        context.bezierCurveTo(35, 12.76, 37.24, 15, 40, 15);
        context.bezierCurveTo(40.73, 15, 41.43, 14.84, 42.06, 14.56);
        context.bezierCurveTo(43.2, 16.41, 44.04, 18.47, 44.53, 20.65);
        context.bezierCurveTo(44.23, 20.82, 43.96, 21.01, 43.71, 21.23);
        context.bezierCurveTo(42.66, 22.15, 42, 23.5, 42, 25);
        context.bezierCurveTo(42, 26.86, 43.02, 28.49, 44.53, 29.35);
        context.bezierCurveTo(44.04, 31.53, 43.2, 33.59, 42.06, 35.44);
        context.bezierCurveTo(41.43, 35.16, 40.73, 35, 40, 35);
        context.bezierCurveTo(39.72, 35, 39.45, 35.02, 39.19, 35.07);
        context.bezierCurveTo(36.81, 35.45, 35, 37.51, 35, 40);
        context.bezierCurveTo(35, 40.73, 35.16, 41.43, 35.44, 42.06);
        context.bezierCurveTo(33.59, 43.2, 31.53, 44.04, 29.35, 44.53);
        context.bezierCurveTo(28.79, 43.56, 27.93, 42.8, 26.89, 42.37);
        context.bezierCurveTo(26.31, 42.13, 25.67, 42, 25, 42);
        context.bezierCurveTo(23.14, 42, 21.51, 43.02, 20.65, 44.53);
        context.bezierCurveTo(18.47, 44.04, 16.41, 43.2, 14.56, 42.06);
        context.bezierCurveTo(14.84, 41.43, 15, 40.73, 15, 40);
        context.bezierCurveTo(15, 38.66, 14.47, 37.44, 13.61, 36.54);
        context.bezierCurveTo(12.7, 35.59, 11.42, 35, 10, 35);
        context.bezierCurveTo(9.27, 35, 8.57, 35.16, 7.94, 35.44);
        context.bezierCurveTo(6.8, 33.59, 5.96, 31.53, 5.47, 29.35);
        context.bezierCurveTo(6.98, 28.49, 8, 26.86, 8, 25);
        context.bezierCurveTo(8, 23.84, 7.6, 22.77, 6.94, 21.92);
        context.bezierCurveTo(6.54, 21.41, 6.04, 20.98, 5.47, 20.65);
        context.bezierCurveTo(5.96, 18.47, 6.8, 16.41, 7.94, 14.56);
        context.bezierCurveTo(8.57, 14.84, 9.27, 15, 10, 15);
        context.bezierCurveTo(12.76, 15, 15, 12.76, 15, 10);
        context.bezierCurveTo(15, 9.42, 14.9, 8.87, 14.72, 8.35);
        context.bezierCurveTo(14.67, 8.21, 14.62, 8.07, 14.56, 7.94);
        context.bezierCurveTo(15.87, 7.13, 17.29, 6.47, 18.78, 5.98);
        context.lineTo(18.89, 5.95);
        context.bezierCurveTo(19.46, 5.77, 20.05, 5.61, 20.65, 5.47);
        context.bezierCurveTo(20.96, 6.02, 21.37, 6.5, 21.86, 6.89);
        context.closePath();
        context.save();
        SurgeVCV.shadow2(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = gradient10(context.createLinearGradient(25, 5.47, 25, 44.53));
        context.fill();

        ////// Bezier Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow11(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();



        //// Oval Drawing
        oval(context, 12, 12, 26, 26);
        context.fillStyle = gradient9(context.createRadialGradient(25, 25, 5.02, 25, 25, 30));
        context.fill();

        ////// Oval Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow10(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.strokeStyle = SurgeVCV.color5;
        context.lineWidth = 2;
        context.stroke();


        //// Rectangle Drawing
        var rectangleCornerRadius = 1;
        var rectangleRect = makeRect(24, 5, 2, 8);
        var rectangleInnerRect = insetRect(rectangleRect, rectangleCornerRadius, rectangleCornerRadius);
        context.beginPath();
        context.arc(rectangleInnerRect.x, rectangleInnerRect.y, rectangleCornerRadius, Math.PI, 1.5*Math.PI);
        context.arc(rectangleInnerRect.x + rectangleInnerRect.w, rectangleInnerRect.y, rectangleCornerRadius, 1.5*Math.PI, 2*Math.PI);
        context.lineTo(rectangleRect.x+rectangleRect.w, rectangleRect.y+rectangleRect.h);
        context.lineTo(rectangleRect.x, rectangleRect.y + rectangleRect.h);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        
        context.restore();

    }

    function drawCanvas3(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 50, 50), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 50, resizedFrame.h / 50);


        //// Color Declarations
        var gradient12Color = 'rgba(21, 21, 21, 1)';
        var gradient12Color2 = 'rgba(84, 84, 84, 1)';
        var gradient13Color = 'rgba(86, 86, 86, 1)';
        var gradient14Color = 'rgba(255, 157, 0, 1)';
        var gradient14Color2 = 'rgba(205, 108, 0, 1)';

        //// Gradient Declarations
        function gradient12(g) {
            g.addColorStop(0, gradient12Color);
            g.addColorStop(1, gradient12Color2);
            return g;
        }
        function gradient13(g) {
            g.addColorStop(0.76, 'rgb(0, 0, 0)');
            g.addColorStop(0.76, blendedColor('rgb(0, 0, 0)', gradient13Color, 0.5));
            g.addColorStop(1, gradient13Color);
            return g;
        }
        function gradient14(g) {
            g.addColorStop(0, gradient14Color);
            g.addColorStop(0.22, blendedColor(gradient14Color, gradient14Color2, 0.5));
            g.addColorStop(0.22, gradient14Color2);
            return g;
        }

        //// Group
        //// Oval Drawing
        oval(context, 10, 13, 30, 30);
        context.fillStyle = gradient12(context.createLinearGradient(25, 43, 25, 13));
        context.fill();


        //// Rectangle Drawing
        context.beginPath();
        context.moveTo(20, 28);
        context.lineTo(16, 45);
        context.lineTo(25, 45);
        context.lineTo(34, 45);
        context.lineTo(30, 28);
        context.lineTo(29, 10);
        context.bezierCurveTo(29, 7.79, 27.21, 6, 25, 6);
        context.lineTo(25, 6);
        context.bezierCurveTo(22.79, 6, 21, 7.79, 21, 10);
        context.lineTo(20, 28);
        context.closePath();
        context.fillStyle = gradient13(context.createLinearGradient(25, 45, 25, 6));
        context.fill();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(25.62, 8.36);
        context.lineTo(25.62, 40.27);
        context.lineTo(24.37, 40.27);
        context.lineTo(24.37, 8.36);
        context.lineTo(25.62, 8.36);
        context.closePath();
        context.fillStyle = gradient14(context.createLinearGradient(25, 8.36, 25, 40.27));
        context.fill();
        
        context.restore();

    }

    function drawSurgeKnobRooster(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 34, 34), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 34, resizedFrame.h / 34);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var gradient12Color = 'rgba(21, 21, 21, 1)';
        var gradient12Color2 = 'rgba(84, 84, 84, 1)';
        var gradient13Color = 'rgba(86, 86, 86, 1)';
        var gradient15Color = 'rgba(177, 177, 177, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient12(g) {
            g.addColorStop(0, gradient12Color);
            g.addColorStop(1, gradient12Color2);
            return g;
        }
        function gradient13(g) {
            g.addColorStop(0.76, 'rgb(0, 0, 0)');
            g.addColorStop(0.76, blendedColor('rgb(0, 0, 0)', gradient13Color, 0.5));
            g.addColorStop(1, gradient13Color);
            return g;
        }
        function gradient15(g) {
            g.addColorStop(0, 'rgb(255, 255, 255)');
            g.addColorStop(0.19, blendedColor('rgb(255, 255, 255)', gradient15Color, 0.5));
            g.addColorStop(0.19, gradient15Color);
            return g;
        }

        //// Group
        //// Oval Drawing
        oval(context, 5, 8, 23, 23);
        context.fillStyle = gradient12(context.createLinearGradient(16.5, 31, 16.5, 8));
        context.fill();


        //// Oval 2 Drawing
        oval(context, 9, 12, 15, 15);
        context.fillStyle = gradient2(context.createLinearGradient(16.5, 12, 16.5, 27));
        context.fill();


        //// Rectangle Drawing
        context.beginPath();
        context.moveTo(12.67, 17.71);
        context.lineTo(9.6, 31.39);
        context.lineTo(16.5, 33);
        context.lineTo(23.4, 31.39);
        context.lineTo(20.33, 17.71);
        context.lineTo(19.57, 3.22);
        context.bezierCurveTo(19.57, 1.44, 18.19, 0, 16.5, 0);
        context.lineTo(16.5, 0);
        context.bezierCurveTo(14.81, -0, 13.43, 1.44, 13.43, 3.22);
        context.lineTo(12.67, 17.71);
        context.closePath();
        context.fillStyle = gradient13(context.createLinearGradient(16.5, 33, 16.5, 0));
        context.fill();


        //// Rectangle 2 Drawing
        roundedRect(context, 16, 2.5, 1, 28.5, 0.5);
        context.fillStyle = gradient15(context.createLinearGradient(16.5, 2.5, 16.5, 31));
        context.fill();
        
        context.restore();

    }

    function drawCanvas5(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 50, 50), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 50, resizedFrame.h / 50);


        //// Color Declarations
        var color26 = 'rgba(38, 38, 38, 1)';
        var gradient16Color = 'rgba(33, 33, 33, 1)';
        var gradient16Color2 = 'rgba(89, 89, 89, 1)';
        var gradient17Color = 'rgba(136, 136, 136, 1)';

        //// Gradient Declarations
        function gradient16(g) {
            g.addColorStop(0, gradient16Color2);
            g.addColorStop(0.44, blendedColor(gradient16Color2, gradient16Color, 0.5));
            g.addColorStop(1, gradient16Color);
            return g;
        }
        function gradient17(g) {
            g.addColorStop(0, gradient17Color);
            g.addColorStop(0.29, blendedColor(gradient17Color, 'rgb(0, 0, 0)', 0.5));
            g.addColorStop(0.53, 'rgb(0, 0, 0)');
            return g;
        }

        //// Polygon Drawing
        context.beginPath();
        context.moveTo(25, 4.5);
        context.lineTo(31.5, 15.75);
        context.lineTo(18.5, 15.75);
        context.closePath();
        context.fillStyle = gradient17(context.createLinearGradient(25, 4.5, 25, 15.75));
        context.fill();


        //// Oval Drawing
        oval(context, 10.5, 10.5, 29, 29);
        context.fillStyle = color26;
        context.fill();


        //// Oval 2 Drawing
        oval(context, 13.5, 13.5, 22.75, 22.75);
        context.fillStyle = gradient16(context.createLinearGradient(24.88, 13.5, 24.88, 36.25));
        context.fill();


        //// Rectangle Drawing
        var rectangleCornerRadius = 0.5;
        var rectangleRect = makeRect(24.5, 7, 1, 18);
        var rectangleInnerRect = insetRect(rectangleRect, rectangleCornerRadius, rectangleCornerRadius);
        context.beginPath();
        context.arc(rectangleInnerRect.x, rectangleInnerRect.y, rectangleCornerRadius, Math.PI, 1.5*Math.PI);
        context.arc(rectangleInnerRect.x + rectangleInnerRect.w, rectangleInnerRect.y, rectangleCornerRadius, 1.5*Math.PI, 2*Math.PI);
        context.lineTo(rectangleRect.x+rectangleRect.w, rectangleRect.y+rectangleRect.h);
        context.lineTo(rectangleRect.x, rectangleRect.y + rectangleRect.h);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        
        context.restore();

    }

    function drawCanvas6(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 50, 50), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 50, resizedFrame.h / 50);


        //// Color Declarations
        var gradient16Color = 'rgba(33, 33, 33, 1)';
        var gradient16Color2 = 'rgba(89, 89, 89, 1)';
        var gradient18Color = 'rgba(49, 49, 49, 1)';

        //// Gradient Declarations
        function gradient16(g) {
            g.addColorStop(0, gradient16Color2);
            g.addColorStop(0.44, blendedColor(gradient16Color2, gradient16Color, 0.5));
            g.addColorStop(1, gradient16Color);
            return g;
        }
        function gradient18(g) {
            g.addColorStop(0, 'rgb(255, 255, 255)');
            g.addColorStop(1, gradient18Color);
            return g;
        }

        //// Oval Drawing
        oval(context, 10.5, 10.5, 29, 29);
        context.fillStyle = gradient18(context.createLinearGradient(25, 10.5, 25, 39.5));
        context.fill();


        //// Oval 2 Drawing
        oval(context, 13.5, 13.5, 22.75, 22.75);
        context.fillStyle = gradient16(context.createLinearGradient(24.88, 13.5, 24.88, 36.25));
        context.fill();


        //// Rectangle Drawing
        var rectangleCornerRadius = 0.5;
        var rectangleRect = makeRect(24.5, 12, 1, 13);
        var rectangleInnerRect = insetRect(rectangleRect, rectangleCornerRadius, rectangleCornerRadius);
        context.beginPath();
        context.arc(rectangleInnerRect.x, rectangleInnerRect.y, rectangleCornerRadius, Math.PI, 1.5*Math.PI);
        context.arc(rectangleInnerRect.x + rectangleInnerRect.w, rectangleInnerRect.y, rectangleCornerRadius, 1.5*Math.PI, 2*Math.PI);
        context.lineTo(rectangleRect.x+rectangleRect.w, rectangleRect.y+rectangleRect.h);
        context.lineTo(rectangleRect.x, rectangleRect.y + rectangleRect.h);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        
        context.restore();

    }

    function drawSurgeKnob_34x34(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 34, 34), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 34, resizedFrame.h / 34);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }

        //// Group
        //// Group 2
        //// Oval 3 Drawing
        oval(context, 0.25, 0.25, 33.5, 33.5);
        context.fillStyle = gradient5(context.createLinearGradient(17, 0.25, 17, 33.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(25.66, 5.66);
        context.bezierCurveTo(25.56, 5.9, 25.5, 6.17, 25.5, 6.46);
        context.bezierCurveTo(25.5, 7.59, 26.42, 8.51, 27.55, 8.51);
        context.bezierCurveTo(27.83, 8.51, 28.11, 8.45, 28.35, 8.34);
        context.bezierCurveTo(29.81, 10.26, 30.8, 12.56, 31.12, 15.07);
        context.bezierCurveTo(30.68, 15.09, 30.28, 15.26, 29.95, 15.52);
        context.bezierCurveTo(29.49, 15.89, 29.19, 16.47, 29.19, 17.11);
        context.bezierCurveTo(29.19, 18.19, 30.01, 19.07, 31.06, 19.16);
        context.bezierCurveTo(30.71, 21.38, 29.84, 23.43, 28.57, 25.18);
        context.bezierCurveTo(28.27, 25, 27.92, 24.9, 27.55, 24.9);
        context.bezierCurveTo(27.39, 24.9, 27.23, 24.92, 27.08, 24.96);
        context.bezierCurveTo(26.17, 25.17, 25.5, 25.98, 25.5, 26.95);
        context.bezierCurveTo(25.5, 27.35, 25.62, 27.73, 25.81, 28.04);
        context.bezierCurveTo(23.87, 29.57, 21.51, 30.6, 18.93, 30.93);
        context.bezierCurveTo(18.88, 30.02, 18.24, 29.28, 17.39, 29.06);
        context.bezierCurveTo(17.23, 29.02, 17.06, 29, 16.89, 29);
        context.bezierCurveTo(15.82, 29, 14.94, 29.82, 14.84, 30.87);
        context.bezierCurveTo(12.54, 30.51, 10.42, 29.58, 8.63, 28.25);
        context.bezierCurveTo(8.92, 27.89, 9.1, 27.44, 9.1, 26.95);
        context.bezierCurveTo(9.1, 26.23, 8.73, 25.6, 8.17, 25.24);
        context.bezierCurveTo(7.85, 25.03, 7.46, 24.9, 7.05, 24.9);
        context.bezierCurveTo(6.55, 24.9, 6.1, 25.07, 5.75, 25.36);
        context.bezierCurveTo(4.4, 23.55, 3.47, 21.41, 3.11, 19.08);
        context.bezierCurveTo(3.96, 18.83, 4.58, 18.05, 4.58, 17.11);
        context.bezierCurveTo(4.58, 16.39, 4.21, 15.75, 3.63, 15.38);
        context.bezierCurveTo(3.46, 15.27, 3.26, 15.19, 3.06, 15.13);
        context.bezierCurveTo(3.38, 12.53, 4.41, 10.15, 5.95, 8.19);
        context.bezierCurveTo(6.27, 8.39, 6.64, 8.51, 7.05, 8.51);
        context.bezierCurveTo(8.18, 8.51, 9.1, 7.59, 9.1, 6.46);
        context.bezierCurveTo(9.1, 6.08, 9, 5.73, 8.82, 5.43);
        context.bezierCurveTo(9.07, 5.25, 9.33, 5.08, 9.59, 4.92);
        context.bezierCurveTo(9.84, 4.76, 10.1, 4.61, 10.36, 4.47);
        context.bezierCurveTo(11.76, 3.71, 13.29, 3.18, 14.92, 2.93);
        context.bezierCurveTo(15.17, 3.78, 15.96, 4.41, 16.89, 4.41);
        context.bezierCurveTo(17.53, 4.41, 18.1, 4.11, 18.48, 3.65);
        context.bezierCurveTo(18.66, 3.43, 18.8, 3.16, 18.87, 2.88);
        context.bezierCurveTo(21.4, 3.19, 23.73, 4.18, 25.66, 5.66);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 2 Drawing
        oval(context, 6.5, 6.5, 21, 21);
        context.fillStyle = gradient2(context.createLinearGradient(17, 6.5, 17, 27.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle Drawing
        roundedRect(context, 15.4, 1.25, 3, 17.25, 1.5);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        
        context.restore();

    }

    function drawSurgeKnobRooster2(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 34, 34), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 34, resizedFrame.h / 34);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var gradient12Color = 'rgba(21, 21, 21, 1)';
        var gradient12Color2 = 'rgba(84, 84, 84, 1)';
        var gradient13Color = 'rgba(86, 86, 86, 1)';
        var gradient15Color = 'rgba(177, 177, 177, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient12(g) {
            g.addColorStop(0, gradient12Color);
            g.addColorStop(1, gradient12Color2);
            return g;
        }
        function gradient13(g) {
            g.addColorStop(0.76, 'rgb(0, 0, 0)');
            g.addColorStop(0.76, blendedColor('rgb(0, 0, 0)', gradient13Color, 0.5));
            g.addColorStop(1, gradient13Color);
            return g;
        }
        function gradient15(g) {
            g.addColorStop(0, 'rgb(255, 255, 255)');
            g.addColorStop(0.19, blendedColor('rgb(255, 255, 255)', gradient15Color, 0.5));
            g.addColorStop(0.19, gradient15Color);
            return g;
        }

        //// Group
        //// Oval Drawing
        oval(context, 3, 4, 27, 27);
        context.fillStyle = gradient12(context.createLinearGradient(16.5, 31, 16.5, 4));
        context.fill();


        //// Oval 2 Drawing
        oval(context, 7, 8, 19, 19);
        context.fillStyle = gradient2(context.createLinearGradient(16.5, 8, 16.5, 27));
        context.fill();


        //// Rectangle Drawing
        context.beginPath();
        context.moveTo(12.67, 17.71);
        context.lineTo(9.6, 31.39);
        context.lineTo(16.5, 33);
        context.lineTo(23.4, 31.39);
        context.lineTo(20.33, 17.71);
        context.lineTo(19.57, 3.22);
        context.bezierCurveTo(19.57, 1.44, 18.19, 0, 16.5, 0);
        context.lineTo(16.5, 0);
        context.bezierCurveTo(14.81, -0, 13.43, 1.44, 13.43, 3.22);
        context.lineTo(12.67, 17.71);
        context.closePath();
        context.fillStyle = gradient13(context.createLinearGradient(16.5, 33, 16.5, 0));
        context.fill();


        //// Rectangle 2 Drawing
        roundedRect(context, 16, 2.5, 1, 28.5, 0.5);
        context.fillStyle = gradient15(context.createLinearGradient(16.5, 2.5, 16.5, 31));
        context.fill();
        
        context.restore();

    }

    function drawADSR(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 110, 380), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 110, resizedFrame.h / 380);
        var resizedShadowScale = Math.min(resizedFrame.w / 110, resizedFrame.h / 380);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';
        var gradient19Color = 'rgba(205, 205, 205, 1)';
        var gradient19Color2 = 'rgba(185, 185, 185, 1)';
        var gradient20Color = 'rgba(16, 53, 99, 1)';
        var gradient20Color2 = 'rgba(9, 99, 163, 1)';
        var gradient20Color3 = 'rgba(50, 133, 253, 1)';
        var fillColor2 = 'rgba(18, 51, 99, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }
        function gradient19(g) {
            g.addColorStop(0, gradient19Color2);
            g.addColorStop(1, gradient19Color);
            return g;
        }
        function gradient20(g) {
            g.addColorStop(0, gradient20Color3);
            g.addColorStop(0.01, blendedColor(gradient20Color3, gradient20Color2, 0.5));
            g.addColorStop(0.01, gradient20Color2);
            g.addColorStop(1, gradient20Color);
            return g;
        }

        //// Rectangle 75 Drawing
        var rectangle75CornerRadius = 2;
        var rectangle75Rect = makeRect(-35.5, 0, 181.5, 321);
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
        context.rect(0, 316, 165, 64);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Group
        context.save();
        context.translate(25, 318);



        //// Rectangle 2 Drawing
        roundedRect(context, -20, 3, 100, 40, 4);
        context.fillStyle = gradient20(context.createLinearGradient(30, 3, 30, 43));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 4
        //// outputText Drawing
        var outputTextRect = makeRect(48, 6, 29, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputTextTotalHeight = 9 * 1.3;
        context.fillText('Output', outputTextRect.x + outputTextRect.w/2, outputTextRect.y + 9 + outputTextRect.h / 2 - outputTextTotalHeight / 2);


        //// connection 4
        //// Oval 13 Drawing
        oval(context, 51, 17, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 14 Drawing
        oval(context, 53, 19, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 15 Drawing
        oval(context, 54, 20, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 16 Drawing
        oval(context, 57, 23, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();






        //// Group 3
        //// connection 3
        //// Oval 9 Drawing
        oval(context, 19, 17, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 10 Drawing
        oval(context, 21, 19, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 11 Drawing
        oval(context, 22, 20, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 12 Drawing
        oval(context, 25, 23, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 2 Drawing
        var outputText2Rect = makeRect(18, 6, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'right';
        var outputText2TotalHeight = 9 * 1.3;
        context.fillText('Retrig', outputText2Rect.x + outputText2Rect.w, outputText2Rect.y + 9 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);




        //// Group 2
        //// connection
        //// Oval Drawing
        oval(context, -13, 17, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 2 Drawing
        oval(context, -11, 19, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 3 Drawing
        oval(context, -10, 20, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 4 Drawing
        oval(context, -7, 23, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 3 Drawing
        var outputText3Rect = makeRect(-14, 6, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText3TotalHeight = 9 * 1.3;
        context.fillText('Gate', outputText3Rect.x + outputText3Rect.w/2, outputText3Rect.y + 9 + outputText3Rect.h / 2 - outputText3TotalHeight / 2);





        context.restore();



        //// Group 5
        //// Oval 6 Drawing
        oval(context, 1.49, 364.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(8.49, 364.49, 8.49, 378.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 6
        context.save();
        context.translate(8.49, 371.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 4 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 7
        //// Oval 5 Drawing
        oval(context, 95.49, 364.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(102.49, 364.49, 102.49, 378.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 8
        context.save();
        context.translate(102.49, 371.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 5 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 6 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 20
        context.save();
        context.translate(51, 89);



        //// Group 48
        //// Rectangle 69 Drawing
        roundedRect(context, -5, -55, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(3.5, -55, 3.5, -19));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 70 Drawing
        context.beginPath();
        context.rect(-3, -53, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 71 Drawing
        context.beginPath();
        context.rect(-3, -50, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 72 Drawing
        context.beginPath();
        context.rect(-3, -47, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 73 Drawing
        context.beginPath();
        context.rect(-3, -44, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// outputText 5 Drawing
        var outputText5Rect = makeRect(-9, -17, 26, 9);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText5TotalHeight = 9 * 1.3;
        context.fillText('Digital', outputText5Rect.x + outputText5Rect.w/2, outputText5Rect.y + 9 + outputText5Rect.h / 2 - outputText5TotalHeight / 2);


        //// outputText 4 Drawing
        var outputText4Rect = makeRect(-10, -66, 29, 9);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText4TotalHeight = 9 * 1.3;
        context.fillText('Analog', outputText4Rect.x + outputText4Rect.w/2, outputText4Rect.y + 9 + outputText4Rect.h / 2 - outputText4TotalHeight / 2);



        context.restore();



        //// Group 25
        //// Group 9
        context.save();
        context.translate(6, 90);



        //// connection 2
        //// Oval 7 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 8 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 17 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 18 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 10
        //// Group 11
        //// Oval 19 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 20 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 7 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 43
        //// Rectangle 44 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 45 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 46 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 47 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 48 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();






        context.restore();



        //// Text Drawing
        var textRect = makeRect(11, 83, 43, 15);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var textTotalHeight = 13 * 1.3;
        context.fillText('Attack', textRect.x + textRect.w/2, textRect.y + 12 + textRect.h / 2 - textTotalHeight / 2);




        //// Group 26
        //// Group 12
        context.save();
        context.translate(6, 147);



        //// connection 5
        //// Oval 21 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 22 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 23 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 24 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 13
        //// Group 14
        //// Oval 25 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier 2 Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 26 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 8 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 15
        //// Rectangle 9 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 10 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 11 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 12 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 13 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();






        context.restore();



        //// Text 2 Drawing
        var text2Rect = makeRect(11, 139, 41, 16);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var text2TotalHeight = 13 * 1.3;
        context.fillText('Decay', text2Rect.x + text2Rect.w/2, text2Rect.y + 12 + text2Rect.h / 2 - text2TotalHeight / 2);




        //// Group 28
        //// Group 16
        context.save();
        context.translate(6, 204);



        //// connection 6
        //// Oval 27 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 28 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 29 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 30 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 17
        //// Group 18
        //// Oval 31 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier 3 Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 32 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 14 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 19
        //// Rectangle 15 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 16 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 17 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 18 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 19 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();






        context.restore();



        //// Text 3 Drawing
        var text3Rect = makeRect(11, 202, 44, 13);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text3TotalHeight = 13 * 1.3;
        context.fillText('Sustain', text3Rect.x, text3Rect.y + 12 + text3Rect.h / 2 - text3TotalHeight / 2);




        //// Group 27
        //// Group 21
        context.save();
        context.translate(6, 261);



        //// connection 7
        //// Oval 33 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 34 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 35 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 36 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 22
        //// Group 23
        //// Oval 37 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier 4 Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 38 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 20 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 24
        //// Rectangle 21 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 22 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 23 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 24 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 25 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();






        context.restore();



        //// Text 4 Drawing
        var text4Rect = makeRect(11, 255, 49, 16);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text4TotalHeight = 13 * 1.3;
        context.fillText('Realase', text4Rect.x, text4Rect.y + 12 + text4Rect.h / 2 - text4TotalHeight / 2);




        //// Group 30
        //// Group 33
        //// Group 34
        //// Bezier 10 Drawing
        context.beginPath();
        context.moveTo(70.51, 2);
        context.lineTo(67.32, 19.27);
        context.lineTo(36.57, 19.27);
        context.lineTo(39.77, 2);
        context.lineTo(70.51, 2);
        context.closePath();
        context.moveTo(66.45, 6.33);
        context.lineTo(58.71, 13.81);
        context.lineTo(55.87, 11.01);
        context.lineTo(49.84, 16.67);
        context.lineTo(46.39, 13.7);
        context.lineTo(42.6, 16.76);
        context.lineTo(41.12, 14.85);
        context.lineTo(46.44, 10.55);
        context.lineTo(49.78, 13.43);
        context.lineTo(55.91, 7.67);
        context.lineTo(58.73, 10.44);
        context.lineTo(64.81, 4.56);
        context.bezierCurveTo(65.36, 5.15, 65.9, 5.74, 66.45, 6.33);
        context.closePath();
        context.fillStyle = fillColor2;
        context.fill();








        //// Group 29
        //// Oval 39 Drawing
        oval(context, 1.49, 1.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(8.49, 1.49, 8.49, 15.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 31
        context.save();
        context.translate(8.49, 8.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 26 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 27 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 32
        //// Oval 40 Drawing
        oval(context, 95.49, 1.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(102.49, 1.49, 102.49, 15.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 35
        context.save();
        context.translate(102.49, 8.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 28 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 29 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();
        
        context.restore();

    }

    function drawCanvas4(canvas, number, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 87, 46), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 87, resizedFrame.h / 46);
        var resizedShadowScale = Math.min(resizedFrame.w / 87, resizedFrame.h / 46);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }

        //// Group
        //// Group 2
        //// Oval 3 Drawing
        oval(context, 38.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(50.5, 11.25, 50.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(56.84, 15.2);
        context.bezierCurveTo(56.76, 15.38, 56.72, 15.58, 56.72, 15.79);
        context.bezierCurveTo(56.72, 16.62, 57.39, 17.29, 58.22, 17.29);
        context.bezierCurveTo(58.42, 17.29, 58.62, 17.25, 58.8, 17.17);
        context.bezierCurveTo(59.87, 18.57, 60.59, 20.25, 60.83, 22.09);
        context.bezierCurveTo(60.51, 22.11, 60.21, 22.23, 59.97, 22.42);
        context.bezierCurveTo(59.63, 22.69, 59.42, 23.11, 59.42, 23.58);
        context.bezierCurveTo(59.42, 24.37, 60.02, 25.01, 60.79, 25.08);
        context.bezierCurveTo(60.53, 26.7, 59.89, 28.2, 58.96, 29.48);
        context.bezierCurveTo(58.74, 29.35, 58.49, 29.28, 58.22, 29.28);
        context.bezierCurveTo(58.1, 29.28, 57.98, 29.29, 57.87, 29.32);
        context.bezierCurveTo(57.21, 29.47, 56.72, 30.07, 56.72, 30.78);
        context.bezierCurveTo(56.72, 31.07, 56.8, 31.35, 56.95, 31.58);
        context.bezierCurveTo(55.52, 32.69, 53.8, 33.44, 51.91, 33.69);
        context.bezierCurveTo(51.88, 33.03, 51.41, 32.48, 50.78, 32.32);
        context.bezierCurveTo(50.67, 32.29, 50.54, 32.28, 50.42, 32.28);
        context.bezierCurveTo(49.63, 32.28, 48.99, 32.88, 48.92, 33.65);
        context.bezierCurveTo(47.24, 33.38, 45.69, 32.7, 44.38, 31.73);
        context.bezierCurveTo(44.59, 31.47, 44.72, 31.14, 44.72, 30.78);
        context.bezierCurveTo(44.72, 30.25, 44.45, 29.79, 44.04, 29.52);
        context.bezierCurveTo(43.81, 29.37, 43.52, 29.28, 43.22, 29.28);
        context.bezierCurveTo(42.86, 29.28, 42.53, 29.41, 42.27, 29.62);
        context.bezierCurveTo(41.28, 28.29, 40.6, 26.73, 40.34, 25.02);
        context.bezierCurveTo(40.97, 24.84, 41.42, 24.26, 41.42, 23.58);
        context.bezierCurveTo(41.42, 23.05, 41.14, 22.58, 40.72, 22.32);
        context.bezierCurveTo(40.6, 22.24, 40.45, 22.17, 40.3, 22.13);
        context.bezierCurveTo(40.54, 20.23, 41.29, 18.49, 42.42, 17.06);
        context.bezierCurveTo(42.65, 17.2, 42.93, 17.29, 43.22, 17.29);
        context.bezierCurveTo(44.05, 17.29, 44.72, 16.62, 44.72, 15.79);
        context.bezierCurveTo(44.72, 15.52, 44.65, 15.26, 44.52, 15.04);
        context.bezierCurveTo(44.7, 14.91, 44.89, 14.78, 45.08, 14.66);
        context.bezierCurveTo(45.26, 14.55, 45.45, 14.44, 45.64, 14.33);
        context.bezierCurveTo(46.67, 13.78, 47.79, 13.39, 48.98, 13.21);
        context.bezierCurveTo(49.16, 13.83, 49.74, 14.29, 50.42, 14.29);
        context.bezierCurveTo(50.89, 14.29, 51.31, 14.07, 51.58, 13.74);
        context.bezierCurveTo(51.71, 13.57, 51.81, 13.38, 51.87, 13.17);
        context.bezierCurveTo(53.72, 13.4, 55.42, 14.13, 56.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 2 Drawing
        oval(context, 42.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(50.5, 15.5, 50.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle Drawing
        roundedRect(context, 49.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 43
        //// Rectangle 44 Drawing
        var rectangle44Rect = makeRect(70, 5, 17, 36);
        roundedRect(context, rectangle44Rect.x, rectangle44Rect.y, rectangle44Rect.w, rectangle44Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle44Rect.x, rectangle44Rect.y, rectangle44Rect.x, rectangle44Rect.y + rectangle44Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 45 Drawing
        context.beginPath();
        context.rect(72, 7, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 46 Drawing
        context.beginPath();
        context.rect(72, 10, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 47 Drawing
        context.beginPath();
        context.rect(72, 13, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 48 Drawing
        context.beginPath();
        context.rect(72, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();





        //// Symbol Drawing
        var symbolRect = makeRect(0, 6, 33, 33);
        context.save();
        context.beginPath();
        context.rect(symbolRect.x, symbolRect.y, symbolRect.w, symbolRect.h);
        context.clip();
        context.translate(symbolRect.x, symbolRect.y);

        SurgeVCV.drawPatchPoint(canvas, 4, makeRect(0, 0, symbolRect.w, symbolRect.h), 'stretch');
        context.restore();
        
        context.restore();

    }

    function drawCanvas7(canvas, number, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 29, 59), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 29, resizedFrame.h / 59);
        var resizedShadowScale = Math.min(resizedFrame.w / 29, resizedFrame.h / 59);


        //// Group 48
        //// Rectangle 69 Drawing
        var rectangle69Rect = makeRect(6, 11, 17, 36);
        roundedRect(context, rectangle69Rect.x, rectangle69Rect.y, rectangle69Rect.w, rectangle69Rect.h, number);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(rectangle69Rect.x, rectangle69Rect.y, rectangle69Rect.x, rectangle69Rect.y + rectangle69Rect.h));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 70 Drawing
        context.beginPath();
        context.rect(8, 13, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 71 Drawing
        context.beginPath();
        context.rect(8, 16, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 72 Drawing
        context.beginPath();
        context.rect(8, 19, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 73 Drawing
        context.beginPath();
        context.rect(8, 22, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();





        //// outputText 5 Drawing
        var outputText5Rect = makeRect(1, 50, 26, 9);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText5TotalHeight = 9 * 1.3;
        context.fillText('Digital', outputText5Rect.x + outputText5Rect.w/2, outputText5Rect.y + 9 + outputText5Rect.h / 2 - outputText5TotalHeight / 2);


        //// outputText 4 Drawing
        var outputText4Rect = makeRect(0, 0, 29, 9);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText4TotalHeight = 9 * 1.3;
        context.fillText('Analog', outputText4Rect.x + outputText4Rect.w/2, outputText4Rect.y + 9 + outputText4Rect.h / 2 - outputText4TotalHeight / 2);
        
        context.restore();

    }

    function drawADSR2(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 110, 380), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 110, resizedFrame.h / 380);
        var resizedShadowScale = Math.min(resizedFrame.w / 110, resizedFrame.h / 380);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';
        var gradient19Color = 'rgba(205, 205, 205, 1)';
        var gradient19Color2 = 'rgba(185, 185, 185, 1)';
        var gradient20Color = 'rgba(16, 53, 99, 1)';
        var gradient20Color2 = 'rgba(9, 99, 163, 1)';
        var gradient20Color3 = 'rgba(50, 133, 253, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }
        function gradient19(g) {
            g.addColorStop(0, gradient19Color2);
            g.addColorStop(1, gradient19Color);
            return g;
        }
        function gradient20(g) {
            g.addColorStop(0, gradient20Color3);
            g.addColorStop(0.01, blendedColor(gradient20Color3, gradient20Color2, 0.5));
            g.addColorStop(0.01, gradient20Color2);
            g.addColorStop(1, gradient20Color);
            return g;
        }

        //// Rectangle 75 Drawing
        var rectangle75CornerRadius = 2;
        var rectangle75Rect = makeRect(-30.5, 0, 181.5, 324);
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
        context.rect(0, 316, 165, 64);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Group
        context.save();
        context.translate(25, 266);



        //// Rectangle 2 Drawing
        roundedRect(context, -20, -1, 100, 45, 4);
        context.fillStyle = gradient20(context.createLinearGradient(30, -1, 30, 44));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 2;
        context.stroke();


        //// Group 4
        //// outputText Drawing
        var outputTextRect = makeRect(48, 5, 29, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputTextTotalHeight = 9 * 1.3;
        context.fillText('Output', outputTextRect.x + outputTextRect.w/2, outputTextRect.y + 9 + outputTextRect.h / 2 - outputTextTotalHeight / 2);


        //// connection 4
        //// Oval 13 Drawing
        oval(context, 51, 16, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 14 Drawing
        oval(context, 53, 18, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 15 Drawing
        oval(context, 54, 19, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 16 Drawing
        oval(context, 57, 22, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();






        //// Group 3
        //// connection 3
        //// Oval 9 Drawing
        oval(context, 19, 16, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 10 Drawing
        oval(context, 21, 18, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 11 Drawing
        oval(context, 22, 19, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 12 Drawing
        oval(context, 25, 22, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 2 Drawing
        var outputText2Rect = makeRect(18, 5, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'right';
        var outputText2TotalHeight = 9 * 1.3;
        context.fillText('Retrig', outputText2Rect.x + outputText2Rect.w, outputText2Rect.y + 9 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);




        //// Group 2
        //// connection
        //// Oval Drawing
        oval(context, -13, 16, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 2 Drawing
        oval(context, -11, 18, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 3 Drawing
        oval(context, -10, 19, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 4 Drawing
        oval(context, -7, 22, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 3 Drawing
        var outputText3Rect = makeRect(-14, 5, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText3TotalHeight = 9 * 1.3;
        context.fillText('Gate', outputText3Rect.x + outputText3Rect.w/2, outputText3Rect.y + 9 + outputText3Rect.h / 2 - outputText3TotalHeight / 2);





        context.restore();



        //// Group 5
        //// Oval 6 Drawing
        oval(context, 1.49, 364.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(8.49, 364.49, 8.49, 378.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 6
        context.save();
        context.translate(8.49, 371.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 4 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 7
        //// Oval 5 Drawing
        oval(context, 95.49, 364.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(102.49, 364.49, 102.49, 378.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 8
        context.save();
        context.translate(102.49, 371.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 5 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 6 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 20
        context.save();
        context.translate(50, 62);



        //// Group 48
        //// Rectangle 69 Drawing
        roundedRect(context, -5, -57, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(3.5, -57, 3.5, -21));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 70 Drawing
        context.beginPath();
        context.rect(-3, -55, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 71 Drawing
        context.beginPath();
        context.rect(-3, -52, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 72 Drawing
        context.beginPath();
        context.rect(-3, -49, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 73 Drawing
        context.beginPath();
        context.rect(-3, -46, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// outputText 5 Drawing
        var outputText5Rect = makeRect(16, -28, 26, 9);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText5TotalHeight = 9 * 1.3;
        context.fillText('Digital', outputText5Rect.x + outputText5Rect.w/2, outputText5Rect.y + 9 + outputText5Rect.h / 2 - outputText5TotalHeight / 2);


        //// outputText 4 Drawing
        var outputText4Rect = makeRect(16, -57, 29, 9);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText4TotalHeight = 9 * 1.3;
        context.fillText('Analog', outputText4Rect.x + outputText4Rect.w/2, outputText4Rect.y + 9 + outputText4Rect.h / 2 - outputText4TotalHeight / 2);



        context.restore();



        //// Group 25
        //// Group 9
        context.save();
        context.translate(6, 58);



        //// connection 2
        //// Oval 7 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 8 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 17 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 18 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 10
        //// Group 11
        //// Oval 19 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 20 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 7 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 43
        //// Rectangle 44 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 45 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 46 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 47 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 48 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();






        context.restore();



        //// Text Drawing
        var textRect = makeRect(-15, 32, 89, 50);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var textTotalHeight = 13 * 1.3;
        context.fillText('Attack', textRect.x + textRect.w/2, textRect.y + 12 + textRect.h / 2 - textTotalHeight / 2);




        //// Symbols
        //// bmp00158
        //// SurgeClassicLogo_White
        //// Group 54
        //// Group- 56
        //// Group 57
        //// Shape 2 Drawing
        context.beginPath();
        context.moveTo(85.22, 331.04);
        context.lineTo(79.2, 362.63);
        context.lineTo(21.31, 362.63);
        context.lineTo(27.32, 331.04);
        context.lineTo(85.22, 331.04);
        context.closePath();
        context.moveTo(77.57, 338.95);
        context.lineTo(62.99, 352.64);
        context.lineTo(57.65, 347.53);
        context.lineTo(46.3, 357.88);
        context.lineTo(39.8, 352.44);
        context.lineTo(32.66, 358.04);
        context.lineTo(29.88, 354.55);
        context.lineTo(39.89, 346.68);
        context.lineTo(46.17, 351.94);
        context.lineTo(57.73, 341.41);
        context.lineTo(63.02, 346.48);
        context.lineTo(74.48, 335.72);
        context.bezierCurveTo(75.51, 336.8, 76.54, 337.87, 77.57, 338.95);
        context.closePath();
        context.fillStyle = SurgeVCV.fillColor;
        context.fill();














        //// Group 26
        //// Group 12
        context.save();
        context.translate(6, 113);



        //// connection 5
        //// Oval 21 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 22 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 23 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 24 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 13
        //// Group 14
        //// Oval 25 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier 2 Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 26 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 8 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 15
        //// Rectangle 9 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 10 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 11 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 12 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 13 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();






        context.restore();



        //// Text 2 Drawing
        var text2Rect = makeRect(-15, 84, 89, 50);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var text2TotalHeight = 13 * 1.3;
        context.fillText('Decay', text2Rect.x + text2Rect.w/2, text2Rect.y + 12 + text2Rect.h / 2 - text2TotalHeight / 2);




        //// Group 16
        context.save();
        context.translate(6, 167);



        //// connection 6
        //// Oval 27 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 28 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 29 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 30 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 17
        //// Group 18
        //// Oval 31 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier 3 Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 32 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 14 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 19
        //// Rectangle 15 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 16 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 17 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 18 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 19 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();






        context.restore();



        //// Text 3 Drawing
        var text3Rect = makeRect(11, 144, 89, 50);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text3TotalHeight = 13 * 1.3;
        context.fillText('Sustain', text3Rect.x, text3Rect.y + 12 + text3Rect.h / 2 - text3TotalHeight / 2);


        //// Group 21
        context.save();
        context.translate(6, 219);



        //// connection 7
        //// Oval 33 Drawing
        oval(context, 5, 11, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 34 Drawing
        oval(context, 7, 13, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 35 Drawing
        oval(context, 8, 14, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 36 Drawing
        oval(context, 11, 17, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 22
        //// Group 23
        //// Oval 37 Drawing
        oval(context, 37.25, 11.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 11.25, 49.5, 35.75));
        context.fill();
        context.strokeStyle = 'rgb(255, 255, 255)';
        context.lineWidth = 1;
        context.stroke();


        //// Bezier 4 Drawing
        context.beginPath();
        context.moveTo(55.84, 15.2);
        context.bezierCurveTo(55.76, 15.38, 55.72, 15.58, 55.72, 15.79);
        context.bezierCurveTo(55.72, 16.62, 56.39, 17.29, 57.22, 17.29);
        context.bezierCurveTo(57.42, 17.29, 57.62, 17.25, 57.8, 17.17);
        context.bezierCurveTo(58.87, 18.57, 59.59, 20.25, 59.83, 22.09);
        context.bezierCurveTo(59.51, 22.11, 59.21, 22.23, 58.97, 22.42);
        context.bezierCurveTo(58.63, 22.69, 58.42, 23.11, 58.42, 23.58);
        context.bezierCurveTo(58.42, 24.37, 59.02, 25.01, 59.79, 25.08);
        context.bezierCurveTo(59.53, 26.7, 58.89, 28.2, 57.96, 29.48);
        context.bezierCurveTo(57.74, 29.35, 57.49, 29.28, 57.22, 29.28);
        context.bezierCurveTo(57.1, 29.28, 56.98, 29.29, 56.87, 29.32);
        context.bezierCurveTo(56.21, 29.47, 55.72, 30.07, 55.72, 30.78);
        context.bezierCurveTo(55.72, 31.07, 55.8, 31.35, 55.95, 31.58);
        context.bezierCurveTo(54.52, 32.69, 52.8, 33.44, 50.91, 33.69);
        context.bezierCurveTo(50.88, 33.03, 50.41, 32.48, 49.78, 32.32);
        context.bezierCurveTo(49.67, 32.29, 49.54, 32.28, 49.42, 32.28);
        context.bezierCurveTo(48.63, 32.28, 47.99, 32.88, 47.92, 33.65);
        context.bezierCurveTo(46.24, 33.38, 44.69, 32.7, 43.38, 31.73);
        context.bezierCurveTo(43.59, 31.47, 43.72, 31.14, 43.72, 30.78);
        context.bezierCurveTo(43.72, 30.25, 43.45, 29.79, 43.04, 29.52);
        context.bezierCurveTo(42.81, 29.37, 42.52, 29.28, 42.22, 29.28);
        context.bezierCurveTo(41.86, 29.28, 41.53, 29.41, 41.27, 29.62);
        context.bezierCurveTo(40.28, 28.29, 39.6, 26.73, 39.34, 25.02);
        context.bezierCurveTo(39.97, 24.84, 40.42, 24.26, 40.42, 23.58);
        context.bezierCurveTo(40.42, 23.05, 40.14, 22.58, 39.72, 22.32);
        context.bezierCurveTo(39.6, 22.24, 39.45, 22.17, 39.3, 22.13);
        context.bezierCurveTo(39.54, 20.23, 40.29, 18.49, 41.42, 17.06);
        context.bezierCurveTo(41.65, 17.2, 41.93, 17.29, 42.22, 17.29);
        context.bezierCurveTo(43.05, 17.29, 43.72, 16.62, 43.72, 15.79);
        context.bezierCurveTo(43.72, 15.52, 43.65, 15.26, 43.52, 15.04);
        context.bezierCurveTo(43.7, 14.91, 43.89, 14.78, 44.08, 14.66);
        context.bezierCurveTo(44.26, 14.55, 44.45, 14.44, 44.64, 14.33);
        context.bezierCurveTo(45.67, 13.78, 46.79, 13.39, 47.98, 13.21);
        context.bezierCurveTo(48.16, 13.83, 48.74, 14.29, 49.42, 14.29);
        context.bezierCurveTo(49.89, 14.29, 50.31, 14.07, 50.58, 13.74);
        context.bezierCurveTo(50.71, 13.57, 50.81, 13.38, 50.87, 13.17);
        context.bezierCurveTo(52.72, 13.4, 54.42, 14.13, 55.84, 15.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 38 Drawing
        oval(context, 41.5, 15.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 15.5, 49.5, 31.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 20 Drawing
        roundedRect(context, 48.4, 12.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 24
        //// Rectangle 21 Drawing
        roundedRect(context, 73, 5, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 5, 81.5, 41));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 22 Drawing
        context.beginPath();
        context.rect(75, 7, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 23 Drawing
        context.beginPath();
        context.rect(75, 10, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 24 Drawing
        context.beginPath();
        context.rect(75, 13, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 25 Drawing
        context.beginPath();
        context.rect(75, 16, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();






        context.restore();



        //// Text 4 Drawing
        var text4Rect = makeRect(9, 194, 89, 50);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text4TotalHeight = 13 * 1.3;
        context.fillText('Realase', text4Rect.x, text4Rect.y + 12 + text4Rect.h / 2 - text4TotalHeight / 2);
        
        context.restore();

    }

    function drawADSR3(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 110, 380), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 110, resizedFrame.h / 380);
        var resizedShadowScale = Math.min(resizedFrame.w / 110, resizedFrame.h / 380);


        //// Color Declarations
        var gradient19Color = 'rgba(205, 205, 205, 1)';
        var gradient19Color2 = 'rgba(185, 185, 185, 1)';
        var gradient20Color = 'rgba(16, 53, 99, 1)';
        var gradient20Color2 = 'rgba(9, 99, 163, 1)';
        var gradient20Color3 = 'rgba(50, 133, 253, 1)';
        var fillColor2 = 'rgba(18, 51, 99, 1)';

        //// Gradient Declarations
        function gradient19(g) {
            g.addColorStop(0, gradient19Color2);
            g.addColorStop(1, gradient19Color);
            return g;
        }
        function gradient20(g) {
            g.addColorStop(0, gradient20Color3);
            g.addColorStop(0.01, blendedColor(gradient20Color3, gradient20Color2, 0.5));
            g.addColorStop(0.01, gradient20Color2);
            g.addColorStop(1, gradient20Color);
            return g;
        }

        //// Rectangle 75 Drawing
        var rectangle75CornerRadius = 2;
        var rectangle75Rect = makeRect(-35.5, 0, 181.5, 321);
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
        context.rect(0, 316, 165, 64);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Group
        context.save();
        context.translate(25, 318);



        //// Rectangle 2 Drawing
        roundedRect(context, -20, 3, 100, 40, 4);
        context.fillStyle = gradient20(context.createLinearGradient(30, 3, 30, 43));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 4
        //// outputText Drawing
        var outputTextRect = makeRect(48, 6, 29, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputTextTotalHeight = 9 * 1.3;
        context.fillText('Output', outputTextRect.x + outputTextRect.w/2, outputTextRect.y + 9 + outputTextRect.h / 2 - outputTextTotalHeight / 2);


        //// connection 4
        //// Oval 13 Drawing
        oval(context, 51, 17, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 14 Drawing
        oval(context, 53, 19, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 15 Drawing
        oval(context, 54, 20, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 16 Drawing
        oval(context, 57, 23, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();






        //// Group 3
        //// connection 3
        //// Oval 9 Drawing
        oval(context, 19, 17, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 10 Drawing
        oval(context, 21, 19, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 11 Drawing
        oval(context, 22, 20, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 12 Drawing
        oval(context, 25, 23, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 2 Drawing
        var outputText2Rect = makeRect(18, 6, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'right';
        var outputText2TotalHeight = 9 * 1.3;
        context.fillText('Retrig', outputText2Rect.x + outputText2Rect.w, outputText2Rect.y + 9 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);




        //// Group 2
        //// connection
        //// Oval Drawing
        oval(context, -13, 17, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 2 Drawing
        oval(context, -11, 19, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 3 Drawing
        oval(context, -10, 20, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 4 Drawing
        oval(context, -7, 23, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 3 Drawing
        var outputText3Rect = makeRect(-14, 6, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText3TotalHeight = 9 * 1.3;
        context.fillText('Gate', outputText3Rect.x + outputText3Rect.w/2, outputText3Rect.y + 9 + outputText3Rect.h / 2 - outputText3TotalHeight / 2);





        context.restore();



        //// Group 5
        //// Oval 6 Drawing
        oval(context, 1.49, 364.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(8.49, 364.49, 8.49, 378.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 6
        context.save();
        context.translate(8.49, 371.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 4 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 7
        //// Oval 5 Drawing
        oval(context, 95.49, 364.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(102.49, 364.49, 102.49, 378.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 8
        context.save();
        context.translate(102.49, 371.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 5 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 6 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 20
        context.save();
        context.translate(51, 89);



        //// Group 48
        //// Rectangle 69 Drawing
        roundedRect(context, -5, -58, 17, 36, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(3.5, -58, 3.5, -22));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 70 Drawing
        context.beginPath();
        context.rect(-3, -56, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 71 Drawing
        context.beginPath();
        context.rect(-3, -53, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 72 Drawing
        context.beginPath();
        context.rect(-3, -50, 13, 2);
        context.fillStyle = SurgeVCV.color6;
        context.fill();


        //// Rectangle 73 Drawing
        context.beginPath();
        context.rect(-3, -47, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.color6;
        context.fill();
        context.restore();





        //// outputText 5 Drawing
        var outputText5Rect = makeRect(17, -30, 26, 9);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText5TotalHeight = 9 * 1.3;
        context.fillText('Digital', outputText5Rect.x + outputText5Rect.w/2, outputText5Rect.y + 9 + outputText5Rect.h / 2 - outputText5TotalHeight / 2);


        //// outputText 4 Drawing
        var outputText4Rect = makeRect(16, -57, 29, 9);
        context.fillStyle = 'rgb(0, 0, 0)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText4TotalHeight = 9 * 1.3;
        context.fillText('Analog', outputText4Rect.x + outputText4Rect.w/2, outputText4Rect.y + 9 + outputText4Rect.h / 2 - outputText4TotalHeight / 2);



        context.restore();



        //// Group 30
        //// Group 33
        //// Group 34
        //// Bezier 10 Drawing
        context.beginPath();
        context.moveTo(70.51, 2);
        context.lineTo(67.32, 19.27);
        context.lineTo(36.57, 19.27);
        context.lineTo(39.77, 2);
        context.lineTo(70.51, 2);
        context.closePath();
        context.moveTo(66.45, 6.33);
        context.lineTo(58.71, 13.81);
        context.lineTo(55.87, 11.01);
        context.lineTo(49.84, 16.67);
        context.lineTo(46.39, 13.7);
        context.lineTo(42.6, 16.76);
        context.lineTo(41.12, 14.85);
        context.lineTo(46.44, 10.55);
        context.lineTo(49.78, 13.43);
        context.lineTo(55.91, 7.67);
        context.lineTo(58.73, 10.44);
        context.lineTo(64.81, 4.56);
        context.bezierCurveTo(65.36, 5.15, 65.9, 5.74, 66.45, 6.33);
        context.closePath();
        context.fillStyle = fillColor2;
        context.fill();








        //// Group 29
        //// Oval 39 Drawing
        oval(context, 1.49, 1.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(8.49, 1.49, 8.49, 15.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 31
        context.save();
        context.translate(8.49, 8.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 26 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 27 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Group 32
        //// Oval 40 Drawing
        oval(context, 95.49, 1.49, 14, 14);
        context.fillStyle = gradient19(context.createLinearGradient(102.49, 1.49, 102.49, 15.49));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 35
        context.save();
        context.translate(102.49, 8.49);
        context.rotate(-135 * Math.PI / 180);



        //// Rectangle 28 Drawing
        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();


        //// Rectangle 29 Drawing
        context.save();
        context.rotate(270 * Math.PI / 180);

        context.beginPath();
        context.rect(-1, -6, 2, 12);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        context.restore();



        context.restore();





        //// Symbol Drawing
        var symbolRect = makeRect(11, 79, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbolRect.x, symbolRect.y, symbolRect.w, symbolRect.h);
        context.clip();
        context.translate(symbolRect.x, symbolRect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbolRect.w, symbolRect.h), 'stretch');
        context.restore();


        //// Symbol 2 Drawing
        var symbol2Rect = makeRect(11, 140, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol2Rect.x, symbol2Rect.y, symbol2Rect.w, symbol2Rect.h);
        context.clip();
        context.translate(symbol2Rect.x, symbol2Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol2Rect.w, symbol2Rect.h), 'stretch');
        context.restore();


        //// Symbol 3 Drawing
        var symbol3Rect = makeRect(11, 202, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol3Rect.x, symbol3Rect.y, symbol3Rect.w, symbol3Rect.h);
        context.clip();
        context.translate(symbol3Rect.x, symbol3Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol3Rect.w, symbol3Rect.h), 'stretch');
        context.restore();


        //// Symbol 4 Drawing
        var symbol4Rect = makeRect(11, 263, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol4Rect.x, symbol4Rect.y, symbol4Rect.w, symbol4Rect.h);
        context.clip();
        context.translate(symbol4Rect.x, symbol4Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol4Rect.w, symbol4Rect.h), 'stretch');
        context.restore();
        
        context.restore();

    }

    function drawCanvas8(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 89, 53), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 89, resizedFrame.h / 53);
        var resizedShadowScale = Math.min(resizedFrame.w / 89, resizedFrame.h / 53);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }

        //// Group 25
        //// Group 9
        context.save();
        context.translate(-5, 12);



        //// connection 2
        //// Oval 7 Drawing
        oval(context, 5, 13, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 8 Drawing
        oval(context, 7, 15, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 17 Drawing
        oval(context, 8, 16, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 18 Drawing
        oval(context, 11, 19, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 10
        //// Group 11
        //// Oval 19 Drawing
        oval(context, 37.25, 13.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 13.25, 49.5, 37.75));
        context.fill();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(55.84, 17.2);
        context.bezierCurveTo(55.76, 17.38, 55.72, 17.58, 55.72, 17.79);
        context.bezierCurveTo(55.72, 18.62, 56.39, 19.29, 57.22, 19.29);
        context.bezierCurveTo(57.42, 19.29, 57.62, 19.25, 57.8, 19.17);
        context.bezierCurveTo(58.87, 20.57, 59.59, 22.25, 59.83, 24.09);
        context.bezierCurveTo(59.51, 24.11, 59.21, 24.23, 58.97, 24.42);
        context.bezierCurveTo(58.63, 24.69, 58.42, 25.11, 58.42, 25.58);
        context.bezierCurveTo(58.42, 26.37, 59.02, 27.01, 59.79, 27.08);
        context.bezierCurveTo(59.53, 28.7, 58.89, 30.2, 57.96, 31.48);
        context.bezierCurveTo(57.74, 31.35, 57.49, 31.28, 57.22, 31.28);
        context.bezierCurveTo(57.1, 31.28, 56.98, 31.29, 56.87, 31.32);
        context.bezierCurveTo(56.21, 31.47, 55.72, 32.07, 55.72, 32.78);
        context.bezierCurveTo(55.72, 33.07, 55.8, 33.35, 55.95, 33.58);
        context.bezierCurveTo(54.52, 34.69, 52.8, 35.44, 50.91, 35.69);
        context.bezierCurveTo(50.88, 35.03, 50.41, 34.48, 49.78, 34.32);
        context.bezierCurveTo(49.67, 34.29, 49.54, 34.28, 49.42, 34.28);
        context.bezierCurveTo(48.63, 34.28, 47.99, 34.88, 47.92, 35.65);
        context.bezierCurveTo(46.24, 35.38, 44.69, 34.7, 43.38, 33.73);
        context.bezierCurveTo(43.59, 33.47, 43.72, 33.14, 43.72, 32.78);
        context.bezierCurveTo(43.72, 32.25, 43.45, 31.79, 43.04, 31.52);
        context.bezierCurveTo(42.81, 31.37, 42.52, 31.28, 42.22, 31.28);
        context.bezierCurveTo(41.86, 31.28, 41.53, 31.41, 41.27, 31.62);
        context.bezierCurveTo(40.28, 30.29, 39.6, 28.73, 39.34, 27.02);
        context.bezierCurveTo(39.97, 26.84, 40.42, 26.26, 40.42, 25.58);
        context.bezierCurveTo(40.42, 25.05, 40.14, 24.58, 39.72, 24.32);
        context.bezierCurveTo(39.6, 24.24, 39.45, 24.17, 39.3, 24.13);
        context.bezierCurveTo(39.54, 22.23, 40.29, 20.49, 41.42, 19.06);
        context.bezierCurveTo(41.65, 19.2, 41.93, 19.29, 42.22, 19.29);
        context.bezierCurveTo(43.05, 19.29, 43.72, 18.62, 43.72, 17.79);
        context.bezierCurveTo(43.72, 17.52, 43.65, 17.26, 43.52, 17.04);
        context.bezierCurveTo(43.7, 16.91, 43.89, 16.78, 44.08, 16.66);
        context.bezierCurveTo(44.26, 16.55, 44.45, 16.44, 44.64, 16.33);
        context.bezierCurveTo(45.67, 15.78, 46.79, 15.39, 47.98, 15.21);
        context.bezierCurveTo(48.16, 15.83, 48.74, 16.29, 49.42, 16.29);
        context.bezierCurveTo(49.89, 16.29, 50.31, 16.07, 50.58, 15.74);
        context.bezierCurveTo(50.71, 15.57, 50.81, 15.38, 50.87, 15.17);
        context.bezierCurveTo(52.72, 15.4, 54.42, 16.13, 55.84, 17.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 20 Drawing
        oval(context, 41.5, 17.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 17.5, 49.5, 33.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 7 Drawing
        roundedRect(context, 48.4, 14.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 43
        //// Rectangle 44 Drawing
        roundedRect(context, 76.5, 13, 17, 24, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(85, 13, 85, 37));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 45 Drawing
        context.beginPath();
        context.rect(78.5, 14.5, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 46 Drawing
        context.beginPath();
        context.rect(78.5, 17.5, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 47 Drawing
        context.beginPath();
        context.rect(78.5, 20.5, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 48 Drawing
        context.beginPath();
        context.rect(78.5, 23.5, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();






        context.restore();





        //// Group 27
        //// Group 36
        //// Group 37
        //// Rectangle 30 Drawing
        roundedRect(context, 0.75, 0.5, 87.5, 19, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(44.5, 0.5, 44.5, 19.5));
        context.fill();
        context.save();
        SurgeVCV.shadow5(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();
        context.restore();


        //// Group 38
        //// outputText 7 Drawing
        var outputText7Rect = makeRect(5.5, 5, 78, 10);
        context.fillStyle = SurgeVCV.color4;
        context.font = '12px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var outputText7TotalHeight = 12 * 1.3;
        context.fillText('Attack : value', outputText7Rect.x, outputText7Rect.y + 11 + outputText7Rect.h / 2 - outputText7TotalHeight / 2);
        
        context.restore();

    }

    function drawLFO(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 310, 380), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 310, resizedFrame.h / 380);
        var resizedShadowScale = Math.min(resizedFrame.w / 310, resizedFrame.h / 380);


        //// Color Declarations
        var gradient2Color2 = 'rgba(255, 175, 95, 1)';
        var gradient2Color4 = 'rgba(207, 123, 0, 1)';
        var gradient2Color7 = 'rgba(255, 146, 19, 1)';
        var color14 = 'rgba(30, 55, 91, 1)';
        var gradient5Color = 'rgba(11, 29, 55, 1)';
        var gradient5Color2 = 'rgba(18, 51, 99, 1)';
        var gradient20Color = 'rgba(16, 53, 99, 1)';
        var gradient20Color2 = 'rgba(9, 99, 163, 1)';
        var gradient20Color3 = 'rgba(50, 133, 253, 1)';
        var fillColor2 = 'rgba(18, 51, 99, 1)';
        var fillColor3 = 'rgba(18, 52, 99, 1)';

        //// Gradient Declarations
        function gradient2(g) {
            g.addColorStop(0, gradient2Color2);
            g.addColorStop(0.49, gradient2Color7);
            g.addColorStop(1, gradient2Color4);
            return g;
        }
        function gradient5(g) {
            g.addColorStop(0, gradient5Color2);
            g.addColorStop(1, gradient5Color);
            return g;
        }
        function gradient20(g) {
            g.addColorStop(0, gradient20Color3);
            g.addColorStop(0.01, blendedColor(gradient20Color3, gradient20Color2, 0.5));
            g.addColorStop(0.01, gradient20Color2);
            g.addColorStop(1, gradient20Color);
            return g;
        }

        //// Rectangle 75 Drawing
        var rectangle75CornerRadius = 2;
        var rectangle75Rect = makeRect(-35.5, 0, 353.5, 321);
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
        context.rect(0, 316, 310, 64);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Group
        context.save();
        context.translate(223, 223);



        //// Rectangle 2 Drawing
        roundedRect(context, -19.5, 101.5, 100, 45, 4);
        context.fillStyle = gradient20(context.createLinearGradient(30.5, 101.5, 30.5, 146.5));
        context.fill();
        context.strokeStyle = SurgeVCV.surgeBlue;
        context.lineWidth = 1;
        context.stroke();


        //// Group 4
        //// outputText Drawing
        var outputTextRect = makeRect(48, 107, 29, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputTextTotalHeight = 9 * 1.3;
        context.fillText('Output', outputTextRect.x + outputTextRect.w/2, outputTextRect.y + 9 + outputTextRect.h / 2 - outputTextTotalHeight / 2);


        //// connection 4
        //// Oval 13 Drawing
        oval(context, 51, 118, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 14 Drawing
        oval(context, 53, 120, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 15 Drawing
        oval(context, 54, 121, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 16 Drawing
        oval(context, 57, 124, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();






        //// Group 3
        //// connection 3
        //// Oval 9 Drawing
        oval(context, 19, 118, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 10 Drawing
        oval(context, 21, 120, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 11 Drawing
        oval(context, 22, 121, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 12 Drawing
        oval(context, 25, 124, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 2 Drawing
        var outputText2Rect = makeRect(18, 107, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'right';
        var outputText2TotalHeight = 9 * 1.3;
        context.fillText('Retrig', outputText2Rect.x + outputText2Rect.w, outputText2Rect.y + 9 + outputText2Rect.h / 2 - outputText2TotalHeight / 2);




        //// Group 2
        //// connection
        //// Oval Drawing
        oval(context, -13, 118, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 2 Drawing
        oval(context, -11, 120, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 3 Drawing
        oval(context, -10, 121, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 4 Drawing
        oval(context, -7, 124, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// outputText 3 Drawing
        var outputText3Rect = makeRect(-14, 107, 24, 9);
        context.fillStyle = 'rgb(255, 255, 255)';
        context.font = '9px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText3TotalHeight = 9 * 1.3;
        context.fillText('Gate', outputText3Rect.x + outputText3Rect.w/2, outputText3Rect.y + 9 + outputText3Rect.h / 2 - outputText3TotalHeight / 2);




        //// Symbol 5 Drawing
        var symbol5Rect = makeRect(-198, 115, 77, 17);
        context.save();
        context.beginPath();
        context.rect(symbol5Rect.x, symbol5Rect.y, symbol5Rect.w, symbol5Rect.h);
        context.clip();
        context.translate(symbol5Rect.x, symbol5Rect.y);

        SurgeVCV.drawButtonBank(canvas, makeRect(0, 0, symbol5Rect.w, symbol5Rect.h), 'stretch');
        context.restore();



        context.restore();



        //// Group 30
        //// Group 33
        //// Group 34
        //// Bezier 10 Drawing
        context.beginPath();
        context.moveTo(167.51, 2);
        context.lineTo(164.32, 19.27);
        context.lineTo(133.57, 19.27);
        context.lineTo(136.77, 2);
        context.lineTo(167.51, 2);
        context.closePath();
        context.moveTo(163.45, 6.33);
        context.lineTo(155.71, 13.81);
        context.lineTo(152.87, 11.01);
        context.lineTo(146.84, 16.67);
        context.lineTo(143.39, 13.7);
        context.lineTo(139.6, 16.76);
        context.lineTo(138.12, 14.85);
        context.lineTo(143.44, 10.55);
        context.lineTo(146.78, 13.43);
        context.lineTo(152.91, 7.67);
        context.lineTo(155.73, 10.44);
        context.lineTo(161.81, 4.56);
        context.bezierCurveTo(162.36, 5.15, 162.9, 5.74, 163.45, 6.33);
        context.closePath();
        context.fillStyle = fillColor2;
        context.fill();








        //// Symbol Drawing
        var symbolRect = makeRect(11, 85, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbolRect.x, symbolRect.y, symbolRect.w, symbolRect.h);
        context.clip();
        context.translate(symbolRect.x, symbolRect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbolRect.w, symbolRect.h), 'stretch');
        context.restore();


        //// Symbol 2 Drawing
        var symbol2Rect = makeRect(11, 144, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol2Rect.x, symbol2Rect.y, symbol2Rect.w, symbol2Rect.h);
        context.clip();
        context.translate(symbol2Rect.x, symbol2Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol2Rect.w, symbol2Rect.h), 'stretch');
        context.restore();


        //// Symbol 3 Drawing
        var symbol3Rect = makeRect(11, 204, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol3Rect.x, symbol3Rect.y, symbol3Rect.w, symbol3Rect.h);
        context.clip();
        context.translate(symbol3Rect.x, symbol3Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol3Rect.w, symbol3Rect.h), 'stretch');
        context.restore();


        //// Symbol 4 Drawing
        var symbol4Rect = makeRect(11, 263, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol4Rect.x, symbol4Rect.y, symbol4Rect.w, symbol4Rect.h);
        context.clip();
        context.translate(symbol4Rect.x, symbol4Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol4Rect.w, symbol4Rect.h), 'stretch');
        context.restore();


        //// Group 5
        context.save();
        context.translate(11, 25);



        //// Group 25
        //// Group 9
        context.save();
        context.translate(-5, 12);



        //// connection 2
        //// Oval 7 Drawing
        oval(context, 5, 13, 23, 23);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();
        context.restore();



        //// Oval 8 Drawing
        oval(context, 7, 15, 19, 19);
        context.fillStyle = 'rgb(85, 85, 85)';
        context.fill();


        //// Oval 17 Drawing
        oval(context, 8, 16, 17, 17);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();


        //// Oval 18 Drawing
        oval(context, 11, 19, 11, 11);
        context.fillStyle = SurgeVCV.color2;
        context.fill();




        //// Group 10
        //// Group 11
        //// Oval 19 Drawing
        oval(context, 37.25, 13.25, 24.5, 24.5);
        context.fillStyle = gradient5(context.createLinearGradient(49.5, 13.25, 49.5, 37.75));
        context.fill();


        //// Bezier Drawing
        context.beginPath();
        context.moveTo(55.84, 17.2);
        context.bezierCurveTo(55.76, 17.38, 55.72, 17.58, 55.72, 17.79);
        context.bezierCurveTo(55.72, 18.62, 56.39, 19.29, 57.22, 19.29);
        context.bezierCurveTo(57.42, 19.29, 57.62, 19.25, 57.8, 19.17);
        context.bezierCurveTo(58.87, 20.57, 59.59, 22.25, 59.83, 24.09);
        context.bezierCurveTo(59.51, 24.11, 59.21, 24.23, 58.97, 24.42);
        context.bezierCurveTo(58.63, 24.69, 58.42, 25.11, 58.42, 25.58);
        context.bezierCurveTo(58.42, 26.37, 59.02, 27.01, 59.79, 27.08);
        context.bezierCurveTo(59.53, 28.7, 58.89, 30.2, 57.96, 31.48);
        context.bezierCurveTo(57.74, 31.35, 57.49, 31.28, 57.22, 31.28);
        context.bezierCurveTo(57.1, 31.28, 56.98, 31.29, 56.87, 31.32);
        context.bezierCurveTo(56.21, 31.47, 55.72, 32.07, 55.72, 32.78);
        context.bezierCurveTo(55.72, 33.07, 55.8, 33.35, 55.95, 33.58);
        context.bezierCurveTo(54.52, 34.69, 52.8, 35.44, 50.91, 35.69);
        context.bezierCurveTo(50.88, 35.03, 50.41, 34.48, 49.78, 34.32);
        context.bezierCurveTo(49.67, 34.29, 49.54, 34.28, 49.42, 34.28);
        context.bezierCurveTo(48.63, 34.28, 47.99, 34.88, 47.92, 35.65);
        context.bezierCurveTo(46.24, 35.38, 44.69, 34.7, 43.38, 33.73);
        context.bezierCurveTo(43.59, 33.47, 43.72, 33.14, 43.72, 32.78);
        context.bezierCurveTo(43.72, 32.25, 43.45, 31.79, 43.04, 31.52);
        context.bezierCurveTo(42.81, 31.37, 42.52, 31.28, 42.22, 31.28);
        context.bezierCurveTo(41.86, 31.28, 41.53, 31.41, 41.27, 31.62);
        context.bezierCurveTo(40.28, 30.29, 39.6, 28.73, 39.34, 27.02);
        context.bezierCurveTo(39.97, 26.84, 40.42, 26.26, 40.42, 25.58);
        context.bezierCurveTo(40.42, 25.05, 40.14, 24.58, 39.72, 24.32);
        context.bezierCurveTo(39.6, 24.24, 39.45, 24.17, 39.3, 24.13);
        context.bezierCurveTo(39.54, 22.23, 40.29, 20.49, 41.42, 19.06);
        context.bezierCurveTo(41.65, 19.2, 41.93, 19.29, 42.22, 19.29);
        context.bezierCurveTo(43.05, 19.29, 43.72, 18.62, 43.72, 17.79);
        context.bezierCurveTo(43.72, 17.52, 43.65, 17.26, 43.52, 17.04);
        context.bezierCurveTo(43.7, 16.91, 43.89, 16.78, 44.08, 16.66);
        context.bezierCurveTo(44.26, 16.55, 44.45, 16.44, 44.64, 16.33);
        context.bezierCurveTo(45.67, 15.78, 46.79, 15.39, 47.98, 15.21);
        context.bezierCurveTo(48.16, 15.83, 48.74, 16.29, 49.42, 16.29);
        context.bezierCurveTo(49.89, 16.29, 50.31, 16.07, 50.58, 15.74);
        context.bezierCurveTo(50.71, 15.57, 50.81, 15.38, 50.87, 15.17);
        context.bezierCurveTo(52.72, 15.4, 54.42, 16.13, 55.84, 17.2);
        context.closePath();
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Oval 20 Drawing
        oval(context, 41.5, 17.5, 16, 16);
        context.fillStyle = gradient2(context.createLinearGradient(49.5, 17.5, 49.5, 33.5));
        context.fill();
        context.strokeStyle = color14;
        context.lineWidth = 0.5;
        context.stroke();




        //// Rectangle 7 Drawing
        roundedRect(context, 48.4, 14.25, 2, 12.25, 1);
        context.fillStyle = SurgeVCV.surgeWhite;
        context.fill();




        //// Group 43
        //// Rectangle 44 Drawing
        roundedRect(context, 73, 13.5, 17, 24, 2);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(81.5, 13.5, 81.5, 37.5));
        context.fill();
        context.strokeStyle = 'rgb(85, 85, 85)';
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 45 Drawing
        context.beginPath();
        context.rect(75, 15, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 46 Drawing
        context.beginPath();
        context.rect(75, 18, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 47 Drawing
        context.beginPath();
        context.rect(75, 21, 13, 2);
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();


        //// Rectangle 48 Drawing
        context.beginPath();
        context.rect(75, 24, 13, 2);
        context.save();
        SurgeVCV.shadow(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = SurgeVCV.surgeOrange;
        context.fill();
        context.restore();






        context.restore();





        //// Group 27
        //// Group 36
        //// Group 37
        //// Rectangle 30 Drawing
        roundedRect(context, 0.75, 0.5, 87.5, 18.5, 4);
        context.fillStyle = SurgeVCV.controlDisplay(context.createLinearGradient(44.5, 0.5, 44.5, 19));
        context.fill();
        context.strokeStyle = SurgeVCV.color;
        context.lineWidth = 1;
        context.lineJoin = 'round';
        context.stroke();


        //// Group 38
        //// outputText 7 Drawing
        var outputText7Rect = makeRect(3, 5, 83, 12);
        context.fillStyle = SurgeVCV.color4;
        context.font = '11px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'center';
        var outputText7TotalHeight = 11 * 1.3;
        context.fillText('Rate : 6.812 Hz', outputText7Rect.x + outputText7Rect.w/2, outputText7Rect.y + 10 + outputText7Rect.h / 2 - outputText7TotalHeight / 2);











        context.restore();



        //// Symbol 6 Drawing
        var symbol6Rect = makeRect(110, 85, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol6Rect.x, symbol6Rect.y, symbol6Rect.w, symbol6Rect.h);
        context.clip();
        context.translate(symbol6Rect.x, symbol6Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol6Rect.w, symbol6Rect.h), 'stretch');
        context.restore();


        //// Symbol 7 Drawing
        var symbol7Rect = makeRect(110, 144, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol7Rect.x, symbol7Rect.y, symbol7Rect.w, symbol7Rect.h);
        context.clip();
        context.translate(symbol7Rect.x, symbol7Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol7Rect.w, symbol7Rect.h), 'stretch');
        context.restore();


        //// Symbol 8 Drawing
        var symbol8Rect = makeRect(110, 204, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol8Rect.x, symbol8Rect.y, symbol8Rect.w, symbol8Rect.h);
        context.clip();
        context.translate(symbol8Rect.x, symbol8Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol8Rect.w, symbol8Rect.h), 'stretch');
        context.restore();


        //// Symbol 9 Drawing
        var symbol9Rect = makeRect(110, 263, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol9Rect.x, symbol9Rect.y, symbol9Rect.w, symbol9Rect.h);
        context.clip();
        context.translate(symbol9Rect.x, symbol9Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol9Rect.w, symbol9Rect.h), 'stretch');
        context.restore();


        //// Symbol 10 Drawing
        var symbol10Rect = makeRect(110, 25, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol10Rect.x, symbol10Rect.y, symbol10Rect.w, symbol10Rect.h);
        context.clip();
        context.translate(symbol10Rect.x, symbol10Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol10Rect.w, symbol10Rect.h), 'stretch');
        context.restore();


        //// Symbol 11 Drawing
        var symbol11Rect = makeRect(209, 85, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol11Rect.x, symbol11Rect.y, symbol11Rect.w, symbol11Rect.h);
        context.clip();
        context.translate(symbol11Rect.x, symbol11Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol11Rect.w, symbol11Rect.h), 'stretch');
        context.restore();


        //// Symbol 12 Drawing
        var symbol12Rect = makeRect(209, 144, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol12Rect.x, symbol12Rect.y, symbol12Rect.w, symbol12Rect.h);
        context.clip();
        context.translate(symbol12Rect.x, symbol12Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol12Rect.w, symbol12Rect.h), 'stretch');
        context.restore();


        //// Symbol 15 Drawing
        var symbol15Rect = makeRect(209, 25, 89, 53);
        context.save();
        context.beginPath();
        context.rect(symbol15Rect.x, symbol15Rect.y, symbol15Rect.w, symbol15Rect.h);
        context.clip();
        context.translate(symbol15Rect.x, symbol15Rect.y);

        SurgeVCV.drawCanvas8(canvas, makeRect(0, 0, symbol15Rect.w, symbol15Rect.h), 'stretch');
        context.restore();


        //// Page-1
        //// Artboard
        //// SURGEFX Drawing
        context.beginPath();
        context.moveTo(172.98, 5.14);
        context.lineTo(172.41, 9.83);
        context.lineTo(177.74, 9.83);
        context.lineTo(177.39, 12.4);
        context.lineTo(172.09, 12.4);
        context.lineTo(171.28, 19);
        context.lineTo(168, 19);
        context.lineTo(170.02, 2.57);
        context.lineTo(179.65, 2.57);
        context.lineTo(179.32, 5.14);
        context.lineTo(172.98, 5.14);
        context.closePath();
        context.moveTo(189.5, 10.08);
        context.lineTo(193.52, 19);
        context.lineTo(190.08, 19);
        context.bezierCurveTo(190.09, 19, 189.81, 18.46, 189.81, 18.46);
        context.lineTo(187.17, 12.11);
        context.bezierCurveTo(187.14, 12.17, 187.11, 12.23, 187.08, 12.28);
        context.bezierCurveTo(187.04, 12.34, 187.01, 12.39, 186.97, 12.44);
        context.lineTo(183.01, 18.46);
        context.bezierCurveTo(183.01, 18.46, 182.63, 19, 182.67, 19);
        context.lineTo(178.75, 19);
        context.lineTo(184.99, 10.22);
        context.lineTo(181.37, 2.57);
        context.lineTo(184.88, 2.57);
        context.bezierCurveTo(184.88, 2.57, 184.97, 2.81, 185.03, 2.96);
        context.lineTo(187.38, 8.41);
        context.bezierCurveTo(187.42, 8.33, 187.46, 8.25, 187.5, 8.18);
        context.bezierCurveTo(187.55, 8.11, 187.6, 8.03, 187.65, 7.95);
        context.lineTo(190.86, 3.01);
        context.bezierCurveTo(190.86, 3.01, 191.13, 2.57, 191.13, 2.57);
        context.lineTo(195, 2.57);
        context.lineTo(189.5, 10.08);
        context.closePath();
        context.fillStyle = fillColor3;
        context.fill();
        
        context.restore();

    }

    function drawButtonBank(canvas, targetFrame, resizing) {
        //// General Declarations
        canvas = initializeCanvas(typeof canvas === 'string' ? document.getElementById(canvas) : canvas);
        var context = canvas.getContext('2d');
        var pixelRatio = canvas.surgeVCVPixelRatio;
        
        //// Resize to Target Frame
        context.save();
        var resizedFrame = applyResizingBehavior(resizing, makeRect(0, 0, 77, 17), targetFrame);
        context.translate(resizedFrame.x, resizedFrame.y);
        context.scale(resizedFrame.w / 77, resizedFrame.h / 17);
        var resizedShadowScale = Math.min(resizedFrame.w / 77, resizedFrame.h / 17);


        //// Color Declarations
        var color30 = 'rgba(189, 188, 170, 1)';
        var color32 = 'rgba(240, 238, 219, 1)';
        var color33 = 'rgba(132, 132, 132, 1)';

        //// Rectangle Drawing
        context.beginPath();
        context.rect(0.5, 0.5, 76, 16);
        context.fillStyle = 'rgb(128, 128, 128)';
        context.fill();

        ////// Rectangle Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow2(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.strokeStyle = color33;
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 2 Drawing
        roundedRect(context, 1.5, 1.5, 14, 14, 1);
        context.save();
        SurgeVCV.shadow12(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = color32;
        context.fill();

        ////// Rectangle 2 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow13(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();

        context.strokeStyle = SurgeVCV.shadow13Color;
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 3 Drawing
        roundedRect(context, 16.5, 1.5, 14, 14, 1);
        context.save();
        SurgeVCV.shadow12(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = color32;
        context.fill();

        ////// Rectangle 3 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow5(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();

        context.strokeStyle = color30;
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 4 Drawing
        roundedRect(context, 31.5, 1.5, 14, 14, 1);
        context.save();
        SurgeVCV.shadow12(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = color32;
        context.fill();

        ////// Rectangle 4 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow5(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();

        context.strokeStyle = color30;
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 5 Drawing
        roundedRect(context, 46.5, 1.5, 14, 14, 1);
        context.save();
        SurgeVCV.shadow12(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = color32;
        context.fill();

        ////// Rectangle 5 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow5(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();

        context.strokeStyle = color30;
        context.lineWidth = 1;
        context.stroke();


        //// Rectangle 6 Drawing
        roundedRect(context, 61.5, 1.5, 14, 14, 1);
        context.save();
        SurgeVCV.shadow12(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = color32;
        context.fill();

        ////// Rectangle 6 Inner Shadow
        context.save();
        context.clip();
        context.moveTo(-10000, -10000);
        context.lineTo(-10000, 10000);
        context.lineTo(10001, 10000);
        context.lineTo(10000, -10000);
        context.closePath();
        SurgeVCV.shadow5(context, pixelRatio);
        context.shadowOffsetX *= resizedShadowScale;
        context.shadowOffsetY *= resizedShadowScale;
        context.shadowBlur *= resizedShadowScale;
        context.fillStyle = 'grey';
        context.fill('evenodd');
        context.restore();
        context.restore();

        context.strokeStyle = color30;
        context.lineWidth = 1;
        context.stroke();


        //// Text 2 Drawing
        var text2Rect = makeRect(5, 3, 7, 12);
        context.fillStyle = SurgeVCV.surgeBlue;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text2TotalHeight = 13 * 1.3;
        context.fillText('1', text2Rect.x, text2Rect.y + 12 + text2Rect.h / 2 - text2TotalHeight / 2);


        //// Text 3 Drawing
        var text3Rect = makeRect(20, 3, 7, 12);
        context.fillStyle = SurgeVCV.surgeOrange3;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text3TotalHeight = 13 * 1.3;
        context.fillText('2', text3Rect.x, text3Rect.y + 12 + text3Rect.h / 2 - text3TotalHeight / 2);


        //// Text 4 Drawing
        var text4Rect = makeRect(35, 3, 7, 12);
        context.fillStyle = SurgeVCV.surgeOrange3;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text4TotalHeight = 13 * 1.3;
        context.fillText('3', text4Rect.x, text4Rect.y + 12 + text4Rect.h / 2 - text4TotalHeight / 2);


        //// Text 5 Drawing
        var text5Rect = makeRect(50, 3, 7, 12);
        context.fillStyle = SurgeVCV.surgeOrange3;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text5TotalHeight = 13 * 1.3;
        context.fillText('4', text5Rect.x, text5Rect.y + 12 + text5Rect.h / 2 - text5TotalHeight / 2);


        //// Text 6 Drawing
        var text6Rect = makeRect(65, 3, 7, 12);
        context.fillStyle = SurgeVCV.surgeOrange3;
        context.font = '13px HelveticaNeue, "Helvetica Neue", Helvetica, Arial, sans-serif';
        context.textAlign = 'left';
        var text6TotalHeight = 13 * 1.3;
        context.fillText('5', text6Rect.x, text6Rect.y + 12 + text6Rect.h / 2 - text6TotalHeight / 2);
        
        context.restore();

    }

    //// Generated Images

    function imageOfKnobControl(pixelRatio) {
        var canvas = document.createElement('canvas');
        canvas.width = 50;
        canvas.height = 50;
        canvas.style.width = canvas.width/pixelRatio + 'px';
        canvas.style.height = canvas.height/pixelRatio + 'px';
        canvas.surgeVCVPixelRatio = pixelRatio;
        var context = canvas.getContext('2d');
        context.scale(pixelRatio, pixelRatio);
        SurgeVCV.drawKnobControl(canvas);
        return canvas;
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
        if ('surgeVCVPixelRatio' in canvas) return canvas;
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
        canvas.surgeVCVPixelRatio = pixelRatio;

        context.scale(pixelRatio, pixelRatio);
        return canvas;
    }

    function blendedColor(color1, color2, ratio) {
        var rgba1 = rgbaComponents(color1);
        var rgba2 = rgbaComponents(color2);
        return makeColor(rgba1[0] * (1-ratio) + rgba2[0] * ratio, rgba1[1] * (1-ratio) + rgba2[1] * ratio, rgba1[2] * (1-ratio) + rgba2[2] * ratio, rgba1[3] * (1-ratio) + rgba2[3] * ratio);
    }

    function makeColor(r, g, b, a) {
        return 'rgba(' + Math.round(255*r) + ', ' + Math.round(255*g) + ', ' + Math.round(255*b) + ', ' + a + ')';
    }

    function rgbaComponents(color) {
        if (color.substring(0, 3) === 'hsl') {
            return RGBAFromHSVA.apply(this, hslaComponents(color));
        }

        if (color.substring(0, 1) === '#') {
            var bytes = color.substring(1).split('');
            if (bytes.length === 3) {
                bytes = [bytes[0], bytes[0], bytes[1], bytes[1], bytes[2], bytes[2]];
            }

            var normHex = '0x' + bytes.join('');
            return [(normHex>>16)&255/255, (normHex>>8)&255/255, normHex&255/255, 1];
        }

        var rgba = color.replace(/[^0-9,\.]/g, '').split(',');
        if (rgba.length === 3) {
            rgba.push(1);
        }

        if (color.indexOf('%') > -1) {
            return [rgba[0] / 100, rgba[1] / 100, rgba[2] / 100, rgba[3]];
        }

        return [rgba[0] / 255, rgba[1] / 255, rgba[2] / 255, rgba[3]];
    }

    function hslaComponents(color) {
        if (color.substring(0, 1) === '#' || color.substring(0, 3) === 'rgb') {
            return HSVAFromRGBA.apply(this, rgbaComponents(color));
        }

        var hsla = color.replace(/[^0-9,\.]/g, '').split(',');
        if (hsla.length < 4) {
            hsla.push(1);
        }

        return [hsla[0] / 360, hsla[1] / 100, hsla[2] / 100, hsla[3]];
    }

    function HSVAFromRGBA(r, g, b, a) {
        var max = Math.max(r, g, b), min = Math.min(r, g, b);
        var delta = max - min;
        var h = 0, s = 0, v = max;

        if (max > 0 && delta > 0.00001) {
            s = delta / max;
            if (r == max) h = (g - b) / delta;
            else if (g == max) h = 2 + (b - r) / delta;
            else if (b == max) h = 4 + (r - g) / delta;
            h /= 6;
            if (h < 0) h += 1;
        }
        return [h, s, v, a];
    }

    function RGBAFromHSVA(h, s, v, a) {
        var r, g, b, i, f, p, q, t;

        i = Math.floor(h * 6);
        f = h * 6 - i;
        p = v * (1 - s);
        q = v * (1 - f * s);
        t = v * (1 - (1 - f) * s);
        switch (i % 6) {
            case 0: r = v, g = t, b = p; break;
            case 1: r = q, g = v, b = p; break;
            case 2: r = p, g = v, b = t; break;
            case 3: r = p, g = q, b = v; break;
            case 4: r = t, g = p, b = v; break;
            case 5: r = v, g = p, b = q; break;
        }
        return [r, g, b, a];
    }

    //// Public Interface

    // Colors
    SurgeVCV.surgeBlue = 'rgba(18, 52, 99, 1)';
    SurgeVCV.surgeWhite = 'rgba(255, 255, 255, 1)';
    SurgeVCV.surgeOrange = 'rgba(230, 130, 0, 1)';
    SurgeVCV.color2 = 'rgba(27, 28, 32, 1)';
    SurgeVCV.color4 = 'rgba(255, 255, 255, 1)';
    SurgeVCV.surgeOrange2 = 'rgba(89, 44, 3, 1)';
    SurgeVCV.surgeOrange3 = 'rgba(200, 99, 6, 1)';
    SurgeVCV.gradient2Color = 'rgba(12, 12, 12, 1)';
    SurgeVCV.gradient2Color3 = 'rgba(29, 29, 29, 1)';
    SurgeVCV.gradient2Color5 = 'rgba(23, 23, 23, 1)';
    SurgeVCV.color = 'rgba(75, 81, 93, 1)';
    SurgeVCV.color5 = 'rgba(0, 133, 230, 1)';
    SurgeVCV.color6 = 'rgba(145, 145, 145, 1)';
    SurgeVCV.fillColor = 'rgba(255, 255, 255, 1)';
    SurgeVCV.color7 = 'rgba(205, 206, 212, 1)';
    SurgeVCV.color9 = 'rgba(156, 157, 160, 1)';
    SurgeVCV.shadow4Color = 'rgba(255, 255, 255, 1)';
    SurgeVCV.shadow5Color = 'rgba(255, 255, 255, 1)';
    SurgeVCV.shadow10Color = 'rgba(255, 255, 255, 1)';
    SurgeVCV.shadow11Color = 'rgba(255, 255, 255, 1)';
    SurgeVCV.shadow12Color = 'rgba(85, 94, 0, 1)';
    SurgeVCV.shadow13Color = 'rgba(255, 138, 0, 1)';

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

    SurgeVCV.shadow3 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 2*pixelRatio;
    context.shadowBlur = 4*pixelRatio;
    context.shadowColor = 'rgba(0, 0, 0, 0.15)';
};

    SurgeVCV.shadow4 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 1*pixelRatio;
    context.shadowBlur = 0*pixelRatio;
    context.shadowColor = 'rgba(255, 255, 255, 0.32)';
};

    SurgeVCV.shadow5 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 0.5*pixelRatio;
    context.shadowColor = SurgeVCV.shadow5Color;
};

    SurgeVCV.shadow6 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 3*pixelRatio;
    context.shadowColor = 'rgb(0, 0, 0)';
};

    SurgeVCV.shadow7 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 1*pixelRatio;
    context.shadowColor = 'rgb(0, 0, 0)';
};

    SurgeVCV.shadow8 = function (context, pixelRatio) {
    context.shadowOffsetX = 3*pixelRatio;
    context.shadowOffsetY = 3*pixelRatio;
    context.shadowBlur = 5*pixelRatio;
    context.shadowColor = 'rgb(0, 0, 0)';
};

    SurgeVCV.shadow9 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 2*pixelRatio;
    context.shadowColor = 'rgb(0, 0, 0)';
};

    SurgeVCV.shadow10 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 2*pixelRatio;
    context.shadowBlur = 3*pixelRatio;
    context.shadowColor = SurgeVCV.shadow10Color;
};

    SurgeVCV.shadow11 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 3*pixelRatio;
    context.shadowColor = 'rgba(255, 255, 255, 0.08)';
};

    SurgeVCV.shadow12 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 1*pixelRatio;
    context.shadowColor = SurgeVCV.shadow12Color;
};

    SurgeVCV.shadow13 = function (context, pixelRatio) {
    context.shadowOffsetX = 0*pixelRatio;
    context.shadowOffsetY = 0*pixelRatio;
    context.shadowBlur = 15*pixelRatio;
    context.shadowColor = SurgeVCV.shadow13Color;
};

    // Drawing Methods
    SurgeVCV.drawSurgeVCVGUI = drawSurgeVCVGUI;
    SurgeVCV.drawPatchPoint = drawPatchPoint;
    SurgeVCV.drawControlTextField = drawControlTextField;
    SurgeVCV.drawInputPatch = drawInputPatch;
    SurgeVCV.drawOutputPatch = drawOutputPatch;
    SurgeVCV.drawKnobControl2 = drawKnobControl2;
    SurgeVCV.drawSurgeKnob = drawSurgeKnob;
    SurgeVCV.drawCanvas1 = drawCanvas1;
    SurgeVCV.drawCanvas2 = drawCanvas2;
    SurgeVCV.drawCanvas3 = drawCanvas3;
    SurgeVCV.drawSurgeKnobRooster = drawSurgeKnobRooster;
    SurgeVCV.drawCanvas5 = drawCanvas5;
    SurgeVCV.drawCanvas6 = drawCanvas6;
    SurgeVCV.drawSurgeKnob_34x34 = drawSurgeKnob_34x34;
    SurgeVCV.drawSurgeKnobRooster2 = drawSurgeKnobRooster2;
    SurgeVCV.drawADSR = drawADSR;
    SurgeVCV.drawCanvas4 = drawCanvas4;
    SurgeVCV.drawCanvas7 = drawCanvas7;
    SurgeVCV.drawADSR2 = drawADSR2;
    SurgeVCV.drawADSR3 = drawADSR3;
    SurgeVCV.drawCanvas8 = drawCanvas8;
    SurgeVCV.drawLFO = drawLFO;
    SurgeVCV.drawButtonBank = drawButtonBank;

    // Generated Images
    SurgeVCV.imageOfKnobControl = imageOfKnobControl;

    // Utilities
    SurgeVCV.clearCanvas = clearCanvas;
    SurgeVCV.makeRect = makeRect;

})();
