/* ============================================================
   SmartPark Frontend – Vanilla JavaScript
   All business logic lives in the C++ backend.
   This module is ONLY responsible for:
     - HTTP requests / JSON handling
     - UI rendering (DOM updates)
     - Input collection + basic client-side validation
   ============================================================ */

'use strict';

// If backend is hosted separately (e.g., frontend on Vercel and backend on Render),
// set your backend URL here (e.g., 'https://smartpark.onrender.com').
// When hosting frontend and C++ backend together on Render/Railway/VPS, leave this empty ('').
const EXTERNAL_BACKEND_URL = '';

const API_BASE = (() => {
  if (EXTERNAL_BACKEND_URL && EXTERNAL_BACKEND_URL.trim() !== '') {
    return EXTERNAL_BACKEND_URL.replace(/\/+$/, '');
  }
  const { protocol, host } = window.location;
  if (protocol === 'file:' || !host) {
    return 'http://127.0.0.1:8080';
  }
  return `${protocol}//${host}`;
})();

/* ------------------------------------------------------------
   Utility helpers
   ------------------------------------------------------------ */
const $  = sel => document.querySelector(sel);
const $$ = sel => Array.from(document.querySelectorAll(sel));

function showLoader()  { $('#globalLoader').hidden = false; }
function hideLoader()  { $('#globalLoader').hidden = true;  }

// Format epoch milliseconds (backend uses ms) to local datetime string
function formatEpochMs(ms) {
  if (!ms) return '—';
  const d = new Date(Number(ms));
  return d.toLocaleString(undefined, {
    year: 'numeric', month: 'short', day: '2-digit',
    hour: '2-digit', minute: '2-digit', second: '2-digit'
  });
}

// Format duration in hours as "Xh Ym" for display
function formatDurationHours(hours) {
  const h = Number(hours) || 0;
  const whole = Math.floor(h);
  const mins = Math.round((h - whole) * 60);
  return `${whole}h ${mins}m`;
}

function escapeHtml(s) {
  return String(s ?? '').replace(/[&<>"']/g, c => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
  }[c]));
}

/* ------------------------------------------------------------
   Toast notifications
   ------------------------------------------------------------ */
function toast(title, body = '', type = 'info', timeoutMs = 4500) {
  const container = $('#toastContainer');
  const el = document.createElement('div');
  el.className = `toast ${type}`;
  const iconChar = type === 'success' ? '✓' : type === 'error' ? '✕' : 'ℹ';
  el.innerHTML = `
    <div class="toast-icon">${iconChar}</div>
    <div>
      <div class="toast-title">${escapeHtml(title)}</div>
      ${body ? `<div class="toast-body">${escapeHtml(body)}</div>` : ''}
    </div>
  `;
  container.appendChild(el);
  setTimeout(() => {
    el.style.transition = '260ms ease';
    el.style.opacity = '0';
    el.style.transform = 'translateX(60px)';
    setTimeout(() => el.remove(), 300);
  }, timeoutMs);
}

/* ------------------------------------------------------------
   Unified API caller
   Wraps fetch with error handling, automatic status update, JSON parsing.
   ------------------------------------------------------------ */
async function apiCall(path, opts = {}) {
  try {
    const url = API_BASE + path;
    const res = await fetch(url, {
      headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' },
      ...opts,
      body: opts.body ? JSON.stringify(opts.body) : undefined
    });
    let data = null;
    try { data = await res.json(); } catch (_) { /* empty body */ }
    if (!res.ok) {
      const msg = (data && data.message) ? data.message : `HTTP ${res.status}`;
      setServerStatus(false, `HTTP ${res.status}`);
      return { ok: false, status: res.status, message: msg, data: data?.data ?? null };
    }
    setServerStatus(true);
    return { ok: true, status: res.status, message: data?.message ?? '', data: data?.data ?? null, raw: data };
  } catch (err) {
    setServerStatus(false, 'Connection failed');
    return { ok: false, status: 0, message: 'Network error – is the C++ backend running?', data: null };
  }
}

function setServerStatus(ok, msg) {
  const dot = $('#statusDot');
  const txt = $('#statusText');
  if (!dot || !txt) return;
  dot.classList.remove('ok', 'bad');
  if (ok === true)      { dot.classList.add('ok');  txt.textContent = 'Server OK'; }
  else if (ok === false){ dot.classList.add('bad'); txt.textContent = msg || 'Server down'; }
  else                  { txt.textContent = 'Connecting…'; }
}

/* ============================================================
   TAB SWITCHING
   ============================================================ */
$$('.tab').forEach(btn => {
  btn.addEventListener('click', () => {
    const name = btn.dataset.tab;
    $$('.tab').forEach(b => b.classList.toggle('active', b === btn));
    $$('.tab-panel').forEach(p => p.classList.toggle('active', p.id === `tab-${name}`));
    // When switching to a data-heavy tab, trigger a refresh.
    if (name === 'layout')   refreshSlots();
    if (name === 'history')  refreshHistory();
    if (name === 'dashboard') refreshDashboard();
  });
});

/* ============================================================
   DASHBOARD REFRESH
   ============================================================ */
async function refreshDashboard() {
  const r = await apiCall('/api/dashboard');
  if (!r.ok) {
    toast('Dashboard failed', r.message, 'error');
    return;
  }
  const d = r.data || {};

  // Lot name
  if (d.lotName) $('#lotName').textContent = d.lotName;

  // Stats
  $('#statTotalSlots').textContent = d.totalSlots ?? 0;
  $('#statOccupied').textContent  = d.occupiedSlots ?? 0;
  $('#statAvailable').textContent = d.availableSlots ?? 0;
  $('#statRevenue').textContent   = Number(d.revenue ?? 0).toFixed(2);
  $('#statServed').textContent    = d.servedVehicles ?? 0;

  // Occupancy percent
  const occ = Number(d.occupiedSlots ?? 0);
  const tot = Number(d.totalSlots ?? 1);
  const pct = tot === 0 ? 0 : Math.round((occ / tot) * 100);
  $('#statOccupiedPct').textContent = `${pct}% utilization`;

  // By-type bar chart
  const byType = d.byType || { car: 0, bike: 0, truck: 0 };
  const cars   = Number(byType.car   || 0);
  const bikes  = Number(byType.bike  || 0);
  const trucks = Number(byType.truck || 0);
  const max = Math.max(cars, bikes, trucks, 1);
  $('#countCar').textContent   = cars;
  $('#countBike').textContent  = bikes;
  $('#countTruck').textContent = trucks;
  $('#barCar').style.width   = `${Math.round((cars   / max) * 100)}%`;
  $('#barBike').style.width  = `${Math.round((bikes  / max) * 100)}%`;
  $('#barTruck').style.width = `${Math.round((trucks / max) * 100)}%`;

  // Recent activity (history records from newest first)
  const list = $('#activityList');
  const items = d.recentActivity || [];
  if (items.length === 0) {
    list.innerHTML = '<p class="muted">No completed parkings yet. Exit a vehicle to see the history here.</p>';
  } else {
    list.innerHTML = items.map(a => {
      const type = String(a.type || '').toLowerCase();
      const icon = type === 'car' ? '🚗' : type === 'bike' ? '🏍️' : '🚚';
      return `
        <div class="activity-item">
          <div class="activity-badge ${type}">${icon}</div>
          <div>
            <div class="activity-title">${escapeHtml(a.registrationNumber || '?')}
              <span class="muted">· ${escapeHtml(a.type || '')} · Slot #${a.slotID ?? '?'}</span>
            </div>
            <div class="activity-meta">Exited ${formatEpochMs(a.exitTimeEpochMs)}</div>
          </div>
          <div class="activity-fee">Rs. ${Number(a.feePaid ?? 0).toFixed(2)}</div>
        </div>`;
    }).join('');
  }
}

/* ============================================================
   VEHICLE REGISTRATION
   ============================================================ */
const registerForm   = $('#registerForm');
const registerBtn    = $('#registerBtn');
const registerResult = $('#registerResult');

registerForm.addEventListener('submit', async () => {
  const type = $('#regType').value.trim();
  let reg   = $('#regNumber').value.trim().toUpperCase();
  reg = reg.replace(/\s+/g, '-'); // normalize spaces to dashes

  registerResult.hidden = true;
  registerResult.className = 'result-box';
  registerResult.innerHTML = '';

  if (!type || !reg) {
    registerResult.hidden = false;
    registerResult.classList.add('error');
    registerResult.textContent = 'Please fill in all fields.';
    return;
  }

  registerBtn.disabled = true;
  registerBtn.textContent = 'Registering…';

  const r = await apiCall('/api/vehicles/register', {
    method: 'POST',
    body: { type, registrationNumber: reg }
  });

  registerBtn.disabled = false;
  registerBtn.textContent = 'Register & Park Vehicle';

  registerResult.hidden = false;
  if (r.ok) {
    registerResult.classList.add('success');
    const d = r.data || {};
    registerResult.innerHTML = `
      <p style="font-weight:700; color:#065f46; margin-bottom:10px;">✅ Vehicle Parked Successfully</p>
      <dl>
        <dt>Registration</dt><dd>${escapeHtml(d.registrationNumber || reg)}</dd>
        <dt>Type</dt><dd>${escapeHtml(d.type || type)}</dd>
        <dt>Assigned Slot</dt><dd>#${d.slotID ?? '?'}</dd>
        <dt>Entry Time</dt><dd>${formatEpochMs(d.entryTimeEpochMs)}</dd>
        <dt>Hourly Rate</dt><dd>Rs. ${Number(d.hourlyRate ?? 0).toFixed(2)}</dd>
      </dl>
      <p class="hint" style="margin-top:12px;">Parking layout and dashboard updated automatically.</p>
    `;
    toast('Vehicle parked', `${type} ${reg} → Slot #${d.slotID ?? '?'}`, 'success');
    // Reset form
    $('#regType').value = '';
    $('#regNumber').value = '';
    // Refresh other UI
    refreshDashboard();
    refreshSlots();
    refreshHistory();
  } else {
    registerResult.classList.add('error');
    registerResult.innerHTML = `<p style="font-weight:700; color:#922b21;">❌ Failed to park</p>
      <p>${escapeHtml(r.message)}</p>`;
    toast('Registration failed', r.message, 'error');
  }
});

/* ============================================================
   PARKING SLOT GRID
   ============================================================ */
async function refreshSlots() {
  const r = await apiCall('/api/slots');
  const grid = $('#slotGrid');
  if (!r.ok) {
    grid.innerHTML = `<p class="muted center">Failed to load slots: ${escapeHtml(r.message)}</p>`;
    return;
  }
  const slots = Array.isArray(r.data) ? r.data : [];
  if (slots.length === 0) {
    grid.innerHTML = '<p class="muted center">No parking slots configured.</p>';
    return;
  }
  grid.innerHTML = slots.map(s => {
    const avail = s.available;
    const typeName = String(s.type || '').toLowerCase();
    const typeClass = typeName.includes('bike') ? 'type-bike'
                    : typeName.includes('truck') ? 'type-truck'
                    : typeName.includes('universal') ? 'type-universal'
                    : 'type-car';
    return `
      <div class="slot-card ${avail ? 'available' : 'occupied'} ${typeClass}">
        <div class="slot-head">
          <div class="slot-id">#${s.id}</div>
          <div class="slot-status">${avail ? 'FREE' : 'FULL'}</div>
        </div>
        <div class="slot-type">${escapeHtml(s.type || '')}</div>
        <div class="slot-vehicle">
          ${avail
            ? '<span class="muted" style="font-size:0.8rem;">— Empty —</span>'
            : `<div>
                 <span class="slot-plate">${escapeHtml(s.parkedReg || '')}</span>
                 <div class="slot-vtype">${escapeHtml(s.parkedType || '')}</div>
               </div>`
          }
        </div>
      </div>`;
  }).join('');
}

/* ============================================================
   EXIT VEHICLE (lookup + confirm)
   ============================================================ */
const exitSearchForm  = $('#exitSearchForm');
const exitSearchBtn   = $('#exitSearchBtn');
const exitSummary     = $('#exitSummary');
const exitConfirmForm = $('#exitConfirmForm');
const confirmExitBtn  = $('#confirmExitBtn');
const exitReceipt     = $('#exitReceipt');

let currentExitReg = '';

exitSearchForm.addEventListener('submit', async () => {
  let reg = $('#exitSearchReg').value.trim().toUpperCase();
  if (!reg) { toast('Missing plate', 'Enter a registration number first.', 'info'); return; }

  exitSummary.hidden = true;
  exitConfirmForm.hidden = true;
  exitReceipt.hidden = true;
  exitSearchBtn.disabled = true;
  exitSearchBtn.textContent = 'Looking up…';

  // Use GET /api/vehicles/{number} to show summary; actual fee calc happens at exit.
  const r = await apiCall(`/api/vehicles/${encodeURIComponent(reg)}`);
  exitSearchBtn.disabled = false;
  exitSearchBtn.textContent = 'Lookup';

  if (!r.ok) {
    exitSummary.hidden = false;
    exitSummary.classList.remove('success');
    exitSummary.classList.add('error');
    exitSummary.innerHTML = `
      <p style="font-weight:700;color:#922b21;">❌ Vehicle not found</p>
      <p>${escapeHtml(r.message)}.</p>
      <p class="hint">Check the plate number, or look it up on the History page (may have already exited).</p>`;
    currentExitReg = '';
    return;
  }

  const d = r.data || {};
  currentExitReg = d.registrationNumber || reg;

  exitSummary.hidden = false;
  exitSummary.className = 'result-box summary-box info';
  exitSummary.innerHTML = `
    <h3 style="margin-bottom:10px;color:var(--navy);">Vehicle Found</h3>
    <dl>
      <dt>Registration</dt><dd>${escapeHtml(d.registrationNumber || reg)}</dd>
      <dt>Vehicle Type</dt><dd>${escapeHtml(d.type || '')}</dd>
      <dt>Parking Slot</dt><dd>#${d.slotID ?? '?'}</dd>
      <dt>Entry Time</dt><dd>${formatEpochMs(d.entryTimeEpochMs)}</dd>
      <dt>Duration so far</dt><dd>${formatDurationHours(d.durationHours)}
        <span class="muted">(${Number(d.durationHours || 0).toFixed(3)} hrs)</span></dd>
      <dt>Hourly Rate</dt><dd>Rs. ${Number(d.hourlyRate || 0).toFixed(2)}</dd>
      <dt>Estimated Fee</dt><dd class="accent">Rs. ${Number(d.currentFee || 0).toFixed(2)} (live)</dd>
    </dl>
    <p class="hint" style="margin-top:10px;">
      ✅ The exact fee is computed by the C++ backend when you confirm exit (polymorphic <code>calculateFee()</code>).
    </p>`;

  exitConfirmForm.hidden = false;
});

exitConfirmForm.addEventListener('submit', async () => {
  if (!currentExitReg) { toast('No vehicle', 'Lookup a vehicle first.', 'info'); return; }

  confirmExitBtn.disabled = true;
  confirmExitBtn.textContent = 'Charging fee & releasing slot…';

  const r = await apiCall('/api/vehicles/exit', {
    method: 'POST',
    body: { registrationNumber: currentExitReg }
  });

  confirmExitBtn.disabled = false;
  confirmExitBtn.textContent = 'Confirm Exit & Charge Fee';

  if (!r.ok) {
    toast('Exit failed', r.message, 'error');
    return;
  }
  const d = r.data || {};
  exitConfirmForm.hidden = true;
  exitSummary.hidden = true;
  exitReceipt.hidden = false;
  exitReceipt.innerHTML = `
    <h3>🧾 Exit Receipt – Paid</h3>
    <dl>
      <dt>Registration</dt><dd>${escapeHtml(d.registrationNumber || currentExitReg)}</dd>
      <dt>Vehicle Type</dt><dd>${escapeHtml(d.type || '')}</dd>
      <dt>Parking Slot</dt><dd>#${d.slotID ?? '?'}</dd>
      <dt>Entry Time</dt><dd>${formatEpochMs(d.entryTimeEpochMs)}</dd>
      <dt>Exit Time</dt><dd>${formatEpochMs(d.exitTimeEpochMs)}</dd>
      <dt>Duration</dt><dd>${formatDurationHours(d.durationHours)}
        <span class="muted">(${Number(d.durationHours || 0).toFixed(3)} hrs)</span></dd>
      <dd class="total">Total Paid: Rs. ${Number(d.feePaid || 0).toFixed(2)}</dd>
    </dl>
    <p class="hint" style="margin-top:12px;">Thank you! Drive safely 🚗✨</p>`;

  toast('Exit completed', `${d.type || ''} ${d.registrationNumber || currentExitReg} – Rs. ${Number(d.feePaid || 0).toFixed(2)}`, 'success');

  // Reset plate field
  $('#exitSearchReg').value = '';
  currentExitReg = '';
  // Refresh affected views
  refreshDashboard();
  refreshSlots();
  refreshHistory();
});

/* ============================================================
   PARKING HISTORY
   ============================================================ */
const historyFilter = $('#historyFilter');
const historySearch = $('#historySearch');
[historyFilter, historySearch].forEach(el =>
  el.addEventListener('input', renderHistory));

let cachedHistory = { completed: [], active: [] };

async function refreshHistory() {
  const r = await apiCall('/api/history');
  if (!r.ok) {
    $('#historyTableBody').innerHTML =
      `<tr><td colspan="8" class="center muted">Failed to load history: ${escapeHtml(r.message)}</td></tr>`;
    return;
  }
  const d = r.data || {};
  cachedHistory.completed = Array.isArray(d.completed) ? d.completed : [];
  cachedHistory.active    = Array.isArray(d.active)    ? d.active    : [];
  renderHistory();
}

function renderHistory() {
  const filter = historyFilter.value;
  const q = historySearch.value.trim().toLowerCase();

  const activeRows = cachedHistory.active.map(v => ({
    ...v,
    _isActive: true,
    _sortKey: Number(v.entryTimeEpochMs || 0)
  }));
  const doneRows = cachedHistory.completed.map(h => ({
    ...h,
    _isActive: false,
    _sortKey: Number(h.exitTimeEpochMs || 0)
  }));

  let rows = [...doneRows, ...activeRows];

  // Filter by type/status
  if (filter === 'active')    rows = rows.filter(r => r._isActive);
  if (filter === 'completed') rows = rows.filter(r => !r._isActive);
  if (filter === 'Car' || filter === 'Bike' || filter === 'Truck') {
    rows = rows.filter(r => (r.type || '') === filter);
  }
  // Search
  if (q) rows = rows.filter(r =>
    String(r.registrationNumber || '').toLowerCase().includes(q));

  // Sort newest first
  rows.sort((a, b) => b._sortKey - a._sortKey);

  const body = $('#historyTableBody');
  if (rows.length === 0) {
    body.innerHTML = `<tr><td colspan="8" class="center muted">No records match this filter.</td></tr>`;
    return;
  }

  body.innerHTML = rows.map(r => {
    const tType = String(r.type || '').toLowerCase();
    const plate = r.registrationNumber || '';
    const exit = r.exitTimeEpochMs || null;
    const fee  = r.feePaid != null ? Number(r.feePaid).toFixed(2) : '—';
    return `
      <tr>
        <td><span class="type-tag ${tType}">${escapeHtml(r.type || '')}</span></td>
        <td class="plate">${escapeHtml(plate)}</td>
        <td>#${r.slotID ?? '?'}</td>
        <td>${formatEpochMs(r.entryTimeEpochMs)}</td>
        <td>${r._isActive ? '<span class="status-pill active">Parked</span>' : formatEpochMs(exit)}</td>
        <td>${formatDurationHours(r.durationHours)}<br>
            <span class="muted" style="font-size:0.75rem;">${Number(r.durationHours || 0).toFixed(3)} hrs</span>
        </td>
        <td class="fee">${r._isActive ? '<span class="muted">(ongoing)</span>' : fee}</td>
        <td>${r._isActive
             ? '<span class="status-pill active">🅿️ Active</span>'
             : '<span class="status-pill done">✅ Completed</span>'}
        </td>
      </tr>`;
  }).join('');
}

/* ============================================================
   INITIAL STARTUP
   ============================================================ */
async function bootstrap() {
  showLoader();
  // First ping health to set server status
  const h = await apiCall('/api/health');
  if (!h.ok) {
    toast('Backend offline',
          'Start SmartParkServer.exe and refresh. See README for instructions.',
          'error', 9000);
  } else {
    if (h.data?.lotName) $('#lotName').textContent = h.data.lotName;
  }
  // Refresh all data tabs
  await Promise.all([
    refreshDashboard(),
    refreshSlots(),
    refreshHistory()
  ]);
  hideLoader();

  // Auto-refresh dashboard & slot layout every 20 seconds to keep live data fresh
  setInterval(() => {
    refreshDashboard().catch(() => {});
    refreshSlots().catch(() => {});
  }, 20000);
}

// Kick off
document.addEventListener('DOMContentLoaded', bootstrap);
