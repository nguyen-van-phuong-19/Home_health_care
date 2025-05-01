import React from 'react';
import './StatCard.css';

const StatCard = ({ title, value, unit, icon, color }) => {
  return (
    <div className="stat-card" style={{ borderColor: color }}>
      <div className="stat-icon" style={{ backgroundColor: color }}>
        {icon}
      </div>
      <div className="stat-content">
        <h3>{title}</h3>
        <p className="stat-value">
          {value} <span className="stat-unit">{unit}</span>
        </p>
      </div>
    </div>
  );
};

export default StatCard;
