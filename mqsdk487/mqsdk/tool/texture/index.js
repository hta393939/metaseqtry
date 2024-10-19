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

    const vs = [
      {"size": 1, "col1": 0xffffff, "col2": 0x000000, "x": 0 / 2, "y": 0 / 2},
      {"size": 2, "col1": 0xff0000, "col2": 0x0000ff, "x": w / 2, "y": 0 / 2},
      {"size": 4, "col1": 0xffffff, "col2": 0xff8000, "x": 0 / 2, "y": h / 2},
      {"size": 8, "col1": 0x00ff00, "col2": 0x000000, "x": w / 2, "y": h / 2},
    ];
    for (const v of vs) { // 背景2
      const data = c.getImageData(0, 0, w, h);
      for (let y = 0; y < h / 2; ++y) {
        for (let x = 0; x < w / 2; ++x) {
          let rx = x + v.x;
          let ry = y + v.y;
          let offset = (rx + ry * w) * 4;
          let bx = Math.floor(rx / v.size);
          let by = Math.floor(ry / v.size);
          let lv = (((bx + by) & 1) !== 0) ? v.col1 : v.col2;
          let r = lv >> 16;
          let g = (lv >> 8) & 0xff;
          let b = lv & 0xff;
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

      c.fillStyle = '#ff9999';
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
