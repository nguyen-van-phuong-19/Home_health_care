import React, { useState } from 'react';
import { Routes, Route } from 'react-router-dom';
import Sidebar from './components/Sidebar';
import Dashboard from './pages/Dashboard';
import HeartRate from './pages/HeartRate';
import SpO2 from './pages/SpO2';
import Calories from './pages/Calories';
import Sleep from './pages/Sleep';
import Location from './pages/Location';
import Settings from './pages/Settings';
import './App.css';

function App() {
  const [userId, setUserId] = useState('default_user');

  return (
    <div className="app">
      <Sidebar />
      <main className="content">
        <Routes>
          <Route path="/" element={<Dashboard userId={userId} />} />
          <Route path="/heart-rate" element={<HeartRate userId={userId} />} />
          <Route path="/spo2" element={<SpO2 userId={userId} />} />
          <Route path="/calories" element={<Calories userId={userId} />} />
          <Route path="/sleep" element={<Sleep userId={userId} />} />
          <Route path="/location" element={<Location userId={userId} />} />
          <Route path="/settings" element={<Settings userId={userId} setUserId={setUserId} />} />
        </Routes>
      </main>
    </div>
  );
}

export default App;
