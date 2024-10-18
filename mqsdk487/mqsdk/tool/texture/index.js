class Misc {
  constructor() {

  }
  init() {

    {
      const canvas = document.getElementById('texture1');
      this.drawTexture1(canvas);
    }
    {
      const canvas = document.getElementById('texture2');
      this.drawTexture2(canvas);
    }

    {
      const canvas = document.getElementById('canvas1');
      this.subDraw(canvas);
      this.subDraw(canvas);
      this.subDraw(canvas);
      this.subDraw(canvas);
    }
  }

  /**
   * 
   * @param {HTMLCanvasElement} canvas 
   */
  drawTexture2(canvas) {
    const w = 512;
    const h = 512;
    canvas.width = w;
    canvas.height = h;
    const c = canvas.getContext('2d');
    { // 背景1
      
    }
    { // 背景2
      const data = c.getImageData(0, 0, w, h);
      for (let y = 0; y < h / 2; ++y) {
        for (let x = 0; x < w / 2; ++x) {
          let offset = (x + y * w) * 4;
          let lv = (((x + y) & 1) !== 0) ? 255 : 0;
          let r = lv;
          let g = lv;
          let b = lv;
          let a = 255;
          data.data[offset] = r;
          data.data[offset+1] = g;
          data.data[offset+2] = b;
          data.data[offset+3] = a;
        }
      }
      c.putImageData(data, 0, 0);
    }
    {
      c.fillStyle = '#000000';
      c.fillRect(0, 0, 4, h);
      c.fillRect(0, 0, w, 4);

      c.fillStyle = '#ffffff';
      c.fillRect(w - 4, 0, 4, h);
      c.fillRect(0, h - 4, w, 4);
    }
    {
      const px = Math.floor(w * 0.5 * 0.9);
      c.font = `normal ${px}px BIZ UDP ゴシック`;
      for (const v of [
        {"text": "龍", "color": "#ff1111",
            "x": w * 1 / 4, "y": h * 1 / 4},
        {"text": "飛", "color": "#3333ff",
            "x": w * 3 / 4, "y": h * 1 / 4},
        {"text": "銀", "color": "#cccccc",
            "x": w * 1 / 4, "y": h * 3 / 4},
        {"text": "桂", "color": "#11ff11",
            "x": w * 3 / 4, "y": h * 3 / 4},
      ]) {
        c.textAlign = 'center';
        c.textBaseline = 'middle';
        c.fillStyle = v.color;
        c.fillText(v.text,
            v.x, v.y,
        );
      }
    }
  }

  /**
   * 
   * @param {HTMLCanvasElement} canvas 
   */
  drawTexture1(canvas) {
    const w = 512;
    const h = 512;
    canvas.width = w;
    canvas.height = h;
    const c = canvas.getContext('2d');
    { // 背景1
      c.fillStyle = '#cccc00';
      c.fillRect(0, 0, w / 2, h / 2);

      c.fillStyle = '#cc6600';
      c.fillRect(w / 2, 0, w / 2, h / 2);

      c.fillStyle = '#f0f0f0';
      c.fillRect(0, h / 2, w / 2, h / 2);

      c.fillStyle = '#008000';
      c.fillRect(w / 2, h / 2, w / 2, h / 2);
    }
    {
      const pad = 8;
      c.fillStyle = '#000000';
      c.fillRect(0, 0, pad, h);
      c.fillRect(0, 0, w, pad);

      c.fillStyle = '#ffcccc';
      c.fillRect(w - pad, 0, pad, h);
      c.fillRect(0, h - pad, w, pad);
    }
    {
      const px = Math.floor(w * 0.5 * 0.9);
      c.font = `normal ${px}px BIZ UDP ゴシック`;
      for (const v of [
        {"text": "龍", "color": "#ff1111",
            "x": w * 1 / 4, "y": h * 1 / 4},
        {"text": "飛", "color": "#3333ff",
            "x": w * 3 / 4, "y": h * 1 / 4},
        {"text": "銀", "color": "#cccccc",
            "x": w * 1 / 4, "y": h * 3 / 4},
        {"text": "桂", "color": "#11ff11",
            "x": w * 3 / 4, "y": h * 3 / 4},
      ]) {
        c.textAlign = 'center';
        c.textBaseline = 'middle';
        c.fillStyle = v.color;
        c.fillText(v.text,
            v.x, v.y,
        );
      }
    }
  }

  /**
   * 
   * @param {HTMLCanvasElement} canvas 
   */
  subDraw(canvas) {
    const w = canvas.width;
    const h = canvas.height;
    const c = canvas.getContext('2d');
    const data = c.getImageData(0, 0, w, h);
    for (let y = 0; y < 0; ++y) {
      for (let x = 0; x < 0; ++x) {
        let offset = (x + y * w) * 4;

        let r = 0;
        let g = 0;
        let b = 0;
        let a = 255;
        data.data[offset] = r;
        data.data[offset+1] = g;
        data.data[offset+2] = b;
        data.data[offset+3] = a;
      }
    }
    c.putImageData(data, 0, 0);
  }
}

const misc = new Misc();
misc.init();
