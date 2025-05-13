import { useRef, useEffect } from 'react';
import * as THREE from 'three';

export default function ThreeScene({ x = 0, y = 0, z = 0, gesture = 'none' }) {
  const mountRef = useRef();
  const cubeRef = useRef();
  const materialRef = useRef();

  useEffect(() => {
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xeeeeee);

    const camera = new THREE.PerspectiveCamera(75, 1, 0.1, 1000);
    camera.position.z = 3;

    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(300, 300);
    mountRef.current.appendChild(renderer.domElement);

    const light = new THREE.AmbientLight(0xffffff, 1);
    scene.add(light);

    const geometry = new THREE.BoxGeometry(1, 1, 1);
    const material = new THREE.MeshStandardMaterial({ color: 0x2194ce });
    materialRef.current = material;

    const cube = new THREE.Mesh(geometry, material);
    cubeRef.current = cube;
    scene.add(cube);

    const animate = () => {
      cube.rotation.x += x / 100;
      cube.rotation.y += y / 100;
      cube.rotation.z += z / 100;
      renderer.render(scene, camera);
      requestAnimationFrame(animate);
    };

    animate();

    return () => {
      mountRef.current.removeChild(renderer.domElement);
    };
  }, []);

  // 🎆 gesture 动效逻辑
  useEffect(() => {
    if (!cubeRef.current || !materialRef.current) return;

    const cube = cubeRef.current;
    const material = materialRef.current;

    if (gesture === 'nod') {
      material.color.set(0xffffff);
    } else if (gesture === 'swipe') {
      material.color.set(0xff4d4d);
      cube.scale.set(1.5, 1.5, 1.5);
    } else if (gesture === 'circle') {
      material.color.set(0x9933ff);
      cube.rotation.z += 1; // 立刻加快旋转
    }

    const reset = setTimeout(() => {
      material.color.set(0x2194ce);
      cube.scale.set(1, 1, 1);
    }, 500); // 动效持续 0.5 秒

    return () => clearTimeout(reset);
  }, [gesture]);

  return <div ref={mountRef} style={{ border: '1px solid #ccc' }} />;
}