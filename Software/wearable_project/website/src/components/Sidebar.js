import React from 'react';
import { Link, useLocation } from 'react-router-dom';
import { FaHome, FaHeartbeat, FaRunning, FaBed, FaMapMarkerAlt, FaCog } from 'react-icons/fa';
import { GiLungs } from 'react-icons/gi';
import './Sidebar.css';

const Sidebar = () => {
  const location = useLocation();
  
  const isActive = (path) => {
    return location.pathname === path ? 'active' : '';
  };

  return (
    <div className="sidebar">
      <div className="logo">
        <h2>Wearable Dashboard</h2>
      </div>
      <nav className="nav-menu">
        <ul>
          <li className={isActive('/')}>
            <Link to="/">
              <FaHome className="icon" />
              <span>Dashboard</span>
            </Link>
          </li>
          <li className={isActive('/heart-rate')}>
            <Link to="/heart-rate">
              <FaHeartbeat className="icon" />
              <span>Heart Rate</span>
            </Link>
          </li>
          <li className={isActive('/spo2')}>
            <Link to="/spo2">
              <GiLungs className="icon" />
              <span>SpO2</span>
            </Link>
          </li>
          <li className={isActive('/calories')}>
            <Link to="/calories">
              <FaRunning className="icon" />
              <span>Calories</span>
            </Link>
          </li>
          <li className={isActive('/sleep')}>
            <Link to="/sleep">
              <FaBed className="icon" />
              <span>Sleep</span>
            </Link>
          </li>
          <li className={isActive('/location')}>
            <Link to="/location">
              <FaMapMarkerAlt className="icon" />
              <span>Location</span>
            </Link>
          </li>
          <li className={isActive('/settings')}>
            <Link to="/settings">
              <FaCog className="icon" />
              <span>Settings</span>
            </Link>
          </li>
        </ul>
      </nav>
      <div className="sidebar-footer">
        <p>Wearable Device v1.0</p>
      </div>
    </div>
  );
};

export default Sidebar;
