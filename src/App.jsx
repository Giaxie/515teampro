import { useEffect, useState, useRef } from 'react';
import { db, ref, onValue } from './firebase';

function App() {
  const [sensorData, setSensorData] = useState({ x: 0, y: 0, z: 0 });
  const [trail, setTrail] = useState([]);
  const canvasRef = useRef();

  // 监听 Firebase 数据
  useEffect(() => {
    const sensorRef = ref(db, 'sensorData');
    onValue(sensorRef, (snapshot) => {
      const data = snapshot.val();
      if (data) setSensorData(data);
    });
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const width = canvas.width = window.innerWidth;
    const height = canvas.height = window.innerHeight;

    // ✅ 修复左右反转（x 取负）+ ✅ 修复上下反转（y 取负）
    const scale = 30;
    let x = -sensorData.x * scale + width / 2;
    let y = -sensorData.y * scale + height / 2;

    x = Math.max(0, Math.min(x, width));
    y = Math.max(0, Math.min(y, height));

    // ✅ 更快的拖尾（更短、更轻）
    setTrail(prev => {
      const updated = [...prev, { x, y }];
      return updated.length > 20 ? updated.slice(-20) : updated;
    });

    ctx.clearRect(0, 0, width, height);

    // ✅ 只用 x/y 判断 speed（去除重力干扰）
    const speed = Math.sqrt(sensorData.x ** 2 + sensorData.y ** 2);

    // ✅ 区间判断
    let color = 'blue';
    if (speed > 4) color = 'red';
    else if (speed > 1.5) color = 'green';

    // ✅ 拖尾球绘制：更快消失、更高透明度起点
    trail.forEach((point, index) => {
      const opacity = (index + 1) / trail.length;
      ctx.fillStyle = color;
      ctx.globalAlpha = opacity * 0.4; // 更透明
      ctx.beginPath();
      ctx.arc(point.x, point.y, 10, 0, Math.PI * 2);
      ctx.fill();
    });

    ctx.globalAlpha = 1;
  }, [sensorData]);

  return (
    <div style={{ margin: 0, padding: 0 }}>
      <canvas
        ref={canvasRef}
        style={{
          position: 'fixed',
          top: 0,
          left: 0,
          display: 'block',
          zIndex: 0,
        }}
      />

      <div style={{
        position: 'fixed',
        top: 20,
        left: 20,
        background: 'rgba(0,0,0,0.6)',
        color: 'white',
        padding: '1rem',
        borderRadius: '10px',
        fontFamily: 'monospace',
        zIndex: 1,
      }}>
        <p>X: {sensorData.x?.toFixed(2)}</p>
        <p>Y: {sensorData.y?.toFixed(2)}</p>
        <p>Z: {sensorData.z?.toFixed(2)}</p>
        <p>Speed (X/Y): {Math.sqrt(sensorData.x ** 2 + sensorData.y ** 2).toFixed(2)}</p>
      </div>
    </div>
  );
}

export default App;