function drawLine(ctx, begin, end, stroke = 'black', width = 1) {
    if (stroke) {
        ctx.strokeStyle = stroke;
    }

    if (width) {
        ctx.lineWidth = width;
    }

    ctx.beginPath();
    ctx.moveTo(...begin);
    ctx.lineTo(...end);
    ctx.stroke();
}

function draw(position, whereat, orientation) {
    const canvas = document.querySelector('#canvas');

    if (canvas.getContext) {
        const ctx = canvas.getContext('2d');
        let r = 30;
        let l = 30;
        let theta = orientation*Math.PI/180
        if (whereat === 0) {
            drawLine(ctx, [position[0] + r * Math.cos(theta), position[1] + r * Math.sin(theta)], [position[0] + r * Math.cos(theta) + l * Math.sin(theta), position[1] + r * Math.sin(theta) - l * Math.cos(theta)]);
        
            drawLine(ctx, [position[0] - r * Math.cos(theta), position[1] - r * Math.sin(theta)], [position[0] - r * Math.cos(theta) + l * Math.sin(theta), position[1] - r * Math.sin(theta) - l * Math.cos(theta)]);
        } 
        else if (whereat === 1) {
            // left junc
            drawLine(ctx, [position[0] - r * Math.cos(theta), position[1] - r * Math.sin(theta)], [position[0] - r * Math.cos(theta) - l * Math.cos(theta), position[1] - r * Math.sin(theta) - l * Math.sin(theta)]);
        
            drawLine(ctx, [position[0] - r * Math.cos(theta) + 2 * r * Math.sin(theta), position[1] - r * Math.sin(theta) - 2 * r * Math.cos(theta)], [position[0] - r * Math.cos(theta) + 2 * r * Math.sin(theta) - l * Math.cos(theta), position[1] - r * Math.sin(theta) - 2 * r * Math.cos(theta) - l * Math.sin(theta)]);
        
            // right junc
            drawLine(ctx, [position[0] + r * Math.cos(theta), position[1] + r * Math.sin(theta)], [position[0] + r * Math.cos(theta) + l * Math.cos(theta), position[1] + r * Math.sin(theta) + l * Math.sin(theta)]);
        
            drawLine(ctx, [position[0] + r * Math.cos(theta) + 2 * r * Math.sin(theta), position[1] + r * Math.sin(theta) - 2 * r * Math.cos(theta)], [position[0] + r * Math.cos(theta) + 2 * r * Math.sin(theta) + l * Math.cos(theta), position[1] + r * Math.sin(theta) - 2 * r * Math.cos(theta) + l * Math.sin(theta)]);
        
            // straight junc
            drawLine(ctx, [position[0] - r * Math.cos(theta) + 2 * r * Math.sin(theta), position[1] - r * Math.sin(theta) - 2 * r * Math.cos(theta)], [position[0] - r * Math.cos(theta) + 2 * r * Math.sin(theta) + l * Math.sin(theta), position[1] - r * Math.sin(theta) - 2 * r * Math.cos(theta) - l * Math.cos(theta)]);
        
            drawLine(ctx, [position[0] + r * Math.cos(theta) + 2 * r * Math.sin(theta), position[1] + r * Math.sin(theta) - 2 * r * Math.cos(theta)], [position[0] + r * Math.cos(theta) + 2 * r * Math.sin(theta) + l * Math.sin(theta), position[1] + r * Math.sin(theta) - 2 * r * Math.cos(theta) - l * Math.cos(theta)]);
        } 
        else if (whereat === 2) {
            drawLine(ctx, [position[0] - r * Math.cos(theta), position[1] - r * Math.sin(theta)], [position[0] + r * Math.cos(theta), position[1] + r * Math.sin(theta)]);
        } 
        else if (whereat === 3) {
            drawLine(ctx, [position[0] + r * Math.cos(theta), position[1] + r * Math.sin(theta)], [position[0] + r * Math.cos(theta) + l * Math.sin(theta), position[1] + r * Math.sin(theta) - l * Math.cos(theta)], "green");
        
            drawLine(ctx, [position[0] - r * Math.cos(theta), position[1] - r * Math.sin(theta)], [position[0] - r * Math.cos(theta) + l * Math.sin(theta), position[1] - r * Math.sin(theta) - l * Math.cos(theta)], "green");
        }
    }
}