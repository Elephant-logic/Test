// Web Audio Wavetable Dual Synth Prototype
// All audio is generated procedurally. No external assets required.

(async function(){
  try{
    const AudioCtx = window.AudioContext || window.webkitAudioContext;
    if(!AudioCtx){
      console.error('Web Audio API not supported in this environment');
      return;
    }
    const ctx = new AudioCtx();

    // state
    const state = {
      sampleRate: ctx.sampleRate,
      polyphony: 12,
      activeNotes: new Map(),
      presets: {},
    };

    // Master nodes
    const masterGain = ctx.createGain();
    masterGain.gain.value = dbToGain(-6);
    const masterPan = ctx.createStereoPanner();
    const masterCompressor = ctx.createDynamicsCompressor();
    masterCompressor.threshold.value = -3;
    masterCompressor.knee.value = 0;
    masterCompressor.ratio.value = 12;
    masterCompressor.attack.value = 0.003;
    masterCompressor.release.value = 0.25;

    masterGain.connect(masterPan);
    masterPan.connect(masterCompressor);
    masterCompressor.connect(ctx.destination);

    // Multiband split
    const lowCrossover = {value:300};
    const highCrossover = {value:3000};

    // band gains and drives (implemented via GainNodes + Waveshaper)
    const lowGainNode = ctx.createGain();
    const midGainNode = ctx.createGain();
    const highGainNode = ctx.createGain();
    lowGainNode.gain.value = 1; midGainNode.gain.value = 1; highGainNode.gain.value = 1;

    // create crossovers with filters
    function makeCrossover(freq){
      const lp = ctx.createBiquadFilter(); lp.type = 'lowpass'; lp.frequency.value = freq; lp.Q.value = 0.707;
      const hp = ctx.createBiquadFilter(); hp.type = 'highpass'; hp.frequency.value = freq; hp.Q.value = 0.707;
      return {lp,hp};
    }

    let crossLM = makeCrossover(lowCrossover.value);
    let crossMH = makeCrossover(highCrossover.value);

    // Bands chain: source -> low band (lpLM) -> lowGain -> masterGain
    //                                      -> mid (hpLM -> lpMH) -> midGain -> masterGain
    //                                      -> high (hpMH) -> highGain -> masterGain

    // We'll maintain an internal mixer node that every voice connects into.
    const mixBus = ctx.createGain();
    mixBus.gain.value = 1;

    // Connect mixBus to crossovers
    function rebuildCrossovers(){
      crossLM = makeCrossover(lowCrossover.value);
      crossMH = makeCrossover(highCrossover.value);

      // disconnect previous
      try{mixBus.disconnect();}catch(e){}

      mixBus.connect(crossLM.lp);
      mixBus.connect(crossLM.hp);

      crossLM.lp.connect(lowGainNode);
      crossLM.hp.connect(crossMH.lp);
      crossMH.lp.connect(midGainNode);
      crossMH.hp.connect(highGainNode);

      lowGainNode.connect(masterGain);
      midGainNode.connect(masterGain);
      highGainNode.connect(masterGain);
    }
    rebuildCrossovers();

    // Simple waveshaper for drive
    function makeDrive(amount){
      const s = ctx.createWaveShaper();
      const k = amount*50;
      const samples = 1024;
      const curve = new Float32Array(samples);
      const deg = Math.PI/180;
      for(let i=0;i<samples;i++){
        const x = (i*2/(samples-1))-1;
        curve[i] = (1 + k)*x / (1 + k*Math.abs(x));
      }
      s.curve = curve;
      return s;
    }

    const lowDriveNode = makeDrive(0.1);
    const midDriveNode = makeDrive(0.1);
    const highDriveNode = makeDrive(0.1);

    // Insert drives between band gains and master
    lowGainNode.disconnect(); midGainNode.disconnect(); highGainNode.disconnect();
    lowGainNode.connect(lowDriveNode); lowDriveNode.connect(masterGain);
    midGainNode.connect(midDriveNode); midDriveNode.connect(masterGain);
    highGainNode.connect(highDriveNode); highDriveNode.connect(masterGain);

    // util: db to gain
    function dbToGain(db){return Math.pow(10, db/20);}

    // Wavetable generation: procedurally generate N-band harmonic wavetables and morph between shapes
    function generateWavetable(type, size=2048, morphSeed=0){
      const real = new Float32Array(size);
      const imag = new Float32Array(size);
      // harmonic content based on type
      for(let n=1;n<size/2;n++){
        let mag = 0;
        if(type==='sine') mag = (n===1?1:0);
        else if(type==='saw') mag = 1/n; // saw partial amplitudes
        else if(type==='square') mag = (n%2===1?1/n:0);
        else if(type==='triangle') mag = (n%2===1?1/(n*n):0);
        else if(type==='custom'){
          // procedural richer custom: combine a few bands with noise-modulated amplitudes
          mag = (Math.sin(n*0.7 + morphSeed)*0.5 + 0.5) * (1/Math.max(1,n*0.5));
        }
        real[n] = 0;
        imag[n] = mag;
      }
      return ctx.createPeriodicWave(real, imag, {disableNormalization:false});
    }

    // Create bank of wavetables for morph crossfading: generate two PeriodicWaves and crossfade
    function makeMorphWaves(type, size, morph){
      const waveA = generateWavetable(type,size, 0);
      const waveB = generateWavetable(type==='custom'? 'saw' : 'custom', size, morph*10);
      return {waveA, waveB, morph};
    }

    // Voice class for polyphony and envelopes
    class Voice{
      constructor(note, velocity, ctx, destination, params){
        this.note = note; this.velocity = velocity; this.ctx = ctx; this.params = params;
        this.oscNodes = [];
        this.oscGains = [];
        this.output = ctx.createGain();
        this.output.gain.value = velocity/127;
        // sub oscillator
        this.subOsc = null; this.subGain = ctx.createGain(); this.subGain.gain.value = params.subLevel;
        this.noise = null; this.noiseGain = ctx.createGain(); this.noiseGain.gain.value = params.noiseLevel;

        // filter envelope and amp envelope nodes
        this.ampGain = ctx.createGain(); this.ampGain.gain.value=0;
        this.filterGain = ctx.createGain(); this.filterGain.gain.value=0;

        // main filter
        this.filter = ctx.createBiquadFilter(); this.filter.type = params.filterMode; this.filter.frequency.value = params.filterFreq; this.filter.Q.value = params.filterQ;

        // connect chain: osc -> mixBus
        this.output.connect(mixBus);
        // filtering per-voice (pre-split) -> output
        this.filter.connect(this.output);
        this.subGain.connect(this.output);
        this.noiseGain.connect(this.output);

        // apply amp ADSR directly controlling ampGain
        this.oscillatorPhaseOffset = 0;
        this.startTime = ctx.currentTime;
        this._stopped = false;

        this.createOscillators();
        this.applyEnvelopes(true);
      }

      createOscillators(){
        const baseFreq = midiToFreq(this.note);
        const oscLevels = [this.params.oscA.level, this.params.oscB.level];

        // Two layered morphing oscillators implemented by having two PeriodicWave-based oscillators per layer and crossfading
        const layers = [0,1];
        layers.forEach(layerIdx => {
          const layer = this.params.layers[layerIdx];
          if(!layer) return;
          const unison = Math.max(1, Math.round(layer.unison));
          // create multiple detuned oscillators
          for(let u=0;u<unison;u++){
            const oA = ctx.createOscillator();
            const oB = ctx.createOscillator();
            oA.setPeriodicWave(layer.waveA); oB.setPeriodicWave(layer.waveB);

            // create gains for morph crossfade and unison panning
            const gA = ctx.createGain(); const gB = ctx.createGain();
            // morph crossfade: gA = 1-morph, gB = morph
            gA.gain.value = 1 - layer.morph; gB.gain.value = layer.morph;

            // unison detune and phase
            const detuneCents = ((u - (unison-1)/2) * layer.detune);
            oA.detune.value = detuneCents; oB.detune.value = detuneCents;

            // phase: Web Audio doesn't provide direct phase offset, emulate with slight delay via DelayNode (cheap)
            const phaseDelay = ctx.createDelay(0.1);
            const phaseSeconds = (layer.phase%360)/360.0 * (1/baseFreq);
            phaseDelay.delayTime.value = phaseSeconds;

            oA.connect(gA); oB.connect(gB);
            gA.connect(phaseDelay); gB.connect(phaseDelay);

            // panning and level
            const pan = ctx.createStereoPanner(); pan.pan.value = (u - (unison-1)/2) * layer.unisonSpread || 0;
            const levelGain = ctx.createGain(); levelGain.gain.value = layer.level/unison;
            phaseDelay.connect(pan); pan.connect(levelGain);

            // connect level into filter
            levelGain.connect(this.filter);

            oA.start(); oB.start();
            this.oscNodes.push(oA,oB);
            this.oscGains.push(gA,gB,levelGain);
          }
        });

        // sub oscillator: a simple low-frequency sine at octave -1
        this.subOsc = ctx.createOscillator(); this.subOsc.type='sine'; this.subOsc.frequency.value = midiToFreq(this.note-12);
        this.subOsc.connect(this.subGain);
        this.subOsc.start();

        // procedural noise: white noise buffer loop
        const bufferSize = 2 * ctx.sampleRate;
        const noiseBuffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
        const data = noiseBuffer.getChannelData(0);
        for(let i=0;i<bufferSize;i++) data[i] = (Math.random()*2-1) * 0.5;
        this.noise = ctx.createBufferSource(); this.noise.buffer = noiseBuffer; this.noise.loop = true; this.noise.connect(this.noiseGain); this.noise.start();

        this.noiseGain.gain.value = this.params.noiseLevel;
        this.subGain.gain.value = this.params.subLevel;

        // set initial filter type and params
        this.filter.type = this.params.filterMode;
        this.filter.frequency.value = this.params.filterFreq;
        this.filter.Q.value = this.params.filterQ;
      }

      applyEnvelopes(attackNow){
        const now = this.ctx.currentTime;
        // Amp ADSR
        const A = this.params.amp.A; const D = this.params.amp.D; const S = this.params.amp.S; const R = this.params.amp.R;
        // schedule amplitude directly on output.gain so it always affects the voice gain
        try{
          this.output.gain.cancelScheduledValues(now);
          this.output.gain.setValueAtTime(0, now);
          const peak = this.velocity/127 || 1.0;
          this.output.gain.linearRampToValueAtTime(peak, now + A);
          this.output.gain.linearRampToValueAtTime(S * peak, now + A + D);
        }catch(e){ /* ignore scheduling errors */ }

        // Filter ADSR as multiplicative factor applied to filter freq
        const fA = this.params.filt.A; const fD = this.params.filt.D; const fS = this.params.filt.S; const fR = this.params.filt.R;
        this.filter.frequency.cancelScheduledValues(now);
        const baseFreq = this.params.filterFreq;
        const envTarget = baseFreq * (1 + this.params.filtEnvAmount);
        this.filter.frequency.setValueAtTime(baseFreq*0.5, now);
        this.filter.frequency.linearRampToValueAtTime(envTarget, now + fA);
        this.filter.frequency.linearRampToValueAtTime(baseFreq*fS, now + fA + fD);
      }

      noteOff(){
        if(this._stopped) return;
        const now = this.ctx.currentTime;
        const R = this.params.amp.R;
        try{
          this.output.gain.cancelScheduledValues(now);
          const current = this.output.gain.value;
          this.output.gain.setValueAtTime(current, now);
          this.output.gain.linearRampToValueAtTime(0, now + R + 0.01);
        }catch(e){}
        // schedule stops
        setTimeout(()=>this.stop(), (R+0.2)*1000);
        this._stopped = true;
      }

      stop(){
        this.oscNodes.forEach(o=>{try{o.stop();}catch(e){};try{o.disconnect();}catch(e){}});
        try{this.subOsc.stop();}catch(e){}
        try{this.noise.stop();}catch(e){}
        try{this.output.disconnect();}catch(e){}
        // remove from active notes map handled externally
      }
    }

    // Parameters & UI binding
    const params = {
      oscA: {wavetype:'saw', level:0.8},
      oscB: {wavetype:'square', level:0.8},
      layers: [null,null],
      subLevel: 0.3,
      noiseLevel: 0.1,
      stereoWidth: 0.6,
      masterPan: 0,
      filterMode: 'lowpass', filterFreq:1500, filterQ:1, filterDrive:0.1,
      amp: {A:0.01,D:0.1,S:0.9,R:0.2},
      filt: {A:0.01,D:0.1,S:0.9,R:0.5},
      filtEnvAmount: 2.0,
    };

    // create morph waves for both oscillators
    function updateLayer(idx){
      const type = params[idx===0? 'oscA' : 'oscB'].wavetype;
      const morphEl = document.querySelector(`.morph[data-osc="${idx}"]`);
      const morph = morphEl ? morphEl.value * 1 : 0;
      const detuneEl = document.querySelector(`.detune[data-osc="${idx}"]`);
      const detune = detuneEl ? detuneEl.value * 1 : 0;
      const unisonEl = document.querySelector(`.unison[data-osc="${idx}"]`);
      const unison = unisonEl ? unisonEl.value * 1 : 1;
      const phaseEl = document.querySelector(`.phase[data-osc="${idx}"]`);
      const phase = phaseEl ? phaseEl.value * 1 : 0;
      const levelEl = document.querySelector(`.level[data-osc="${idx}"]`);
      const level = levelEl ? levelEl.value * 1 : 0.8;
      const layer = makeMorphWaves(type, 2048, morph);
      layer.morph = morph; layer.detune = detune; layer.unison = unison; layer.phase = phase; layer.level = level; layer.unisonSpread = document.getElementById('stereoWidth') ? document.getElementById('stereoWidth').value*0.5 : 0.25;
      params.layers[idx] = layer;
    }
    updateLayer(0); updateLayer(1);

    // helpers for UI elements
    document.querySelectorAll('.wavetype').forEach(s=>s.addEventListener('change', e=>{
      const idx = +e.target.dataset.osc; params[idx===0? 'oscA' : 'oscB'].wavetype = e.target.value; updateLayer(idx);
    }));
    document.querySelectorAll('.morph').forEach(r=>r.addEventListener('input', e=>{ updateLayer(+e.target.dataset.osc); }));
    document.querySelectorAll('.detune').forEach(r=>r.addEventListener('input', e=>{ updateLayer(+e.target.dataset.osc); }));
    document.querySelectorAll('.unison').forEach(i=>i.addEventListener('change', e=>{ updateLayer(+e.target.dataset.osc); }));
    document.querySelectorAll('.phase').forEach(r=>r.addEventListener('input', e=>{ updateLayer(+e.target.dataset.osc); }));
    document.querySelectorAll('.level').forEach(r=>r.addEventListener('input', e=>{ updateLayer(+e.target.dataset.osc); }));

    const subLevelEl = document.getElementById('subLevel'); if(subLevelEl) subLevelEl.addEventListener('input', e=>{ params.subLevel = +e.target.value; });
    const noiseLevelEl = document.getElementById('noiseLevel'); if(noiseLevelEl) noiseLevelEl.addEventListener('input', e=>{ params.noiseLevel = +e.target.value; });
    const stereoWidthEl = document.getElementById('stereoWidth'); if(stereoWidthEl) stereoWidthEl.addEventListener('input', e=>{ params.stereoWidth = +e.target.value; updateLayer(0); updateLayer(1); });
    const masterPanEl = document.getElementById('masterPan'); if(masterPanEl) masterPanEl.addEventListener('input', e=>{ masterPan.pan.value = +e.target.value; params.masterPan = +e.target.value; });

    const filterModeEl = document.getElementById('filterMode'); if(filterModeEl) filterModeEl.addEventListener('change', e=>{ params.filterMode = e.target.value; });
    const filterFreqEl = document.getElementById('filterFreq'); if(filterFreqEl) filterFreqEl.addEventListener('input', e=>{ params.filterFreq = +e.target.value; });
    const filterQEl = document.getElementById('filterQ'); if(filterQEl) filterQEl.addEventListener('input', e=>{ params.filterQ = +e.target.value; });
    const filterDriveEl = document.getElementById('filterDrive'); if(filterDriveEl) filterDriveEl.addEventListener('input', e=>{ params.filterDrive = +e.target.value; });

    const ampAEl = document.getElementById('ampA'); if(ampAEl) ampAEl.addEventListener('input', e=>{ params.amp.A = +e.target.value; });
    const ampDEl = document.getElementById('ampD'); if(ampDEl) ampDEl.addEventListener('input', e=>{ params.amp.D = +e.target.value; });
    const ampSEl = document.getElementById('ampS'); if(ampSEl) ampSEl.addEventListener('input', e=>{ params.amp.S = +e.target.value; });
    const ampREl = document.getElementById('ampR'); if(ampREl) ampREl.addEventListener('input', e=>{ params.amp.R = +e.target.value; });
    const filtAEl = document.getElementById('filtA'); if(filtAEl) filtAEl.addEventListener('input', e=>{ params.filt.A = +e.target.value; });
    const filtDEl = document.getElementById('filtD'); if(filtDEl) filtDEl.addEventListener('input', e=>{ params.filt.D = +e.target.value; });
    const filtSEl = document.getElementById('filtS'); if(filtSEl) filtSEl.addEventListener('input', e=>{ params.filt.S = +e.target.value; });
    const filtREl = document.getElementById('filtR'); if(filtREl) filtREl.addEventListener('input', e=>{ params.filt.R = +e.target.value; });

    // LFOs using OscillatorNodes controlling AudioParams where possible
    const lfo1 = ctx.createOscillator(); const lfo1Gain = ctx.createGain();
    const lfo2 = ctx.createOscillator(); const lfo2Gain = ctx.createGain();
    lfo1.type = 'sine'; lfo2.type = 'sine';
    lfo1.frequency.value = 3; lfo2.frequency.value = 0.2;
    lfo1Gain.gain.value = 0.2; lfo2Gain.gain.value = 0.5;
    lfo1.connect(lfo1Gain); lfo2.connect(lfo2Gain);
    lfo1.start(); lfo2.start();

    const lfo1rateEl = document.getElementById('lfo1rate'); if(lfo1rateEl) lfo1rateEl.addEventListener('input', e=>{ if(lfo1) lfo1.frequency.value = +e.target.value; });
    const lfo2rateEl = document.getElementById('lfo2rate'); if(lfo2rateEl) lfo2rateEl.addEventListener('input', e=>{ if(lfo2) lfo2.frequency.value = +e.target.value; });
    const lfo1depthEl = document.getElementById('lfo1depth'); if(lfo1depthEl) lfo1depthEl.addEventListener('input', e=>{ if(lfo1Gain) lfo1Gain.gain.value = +e.target.value; });
    const lfo2depthEl = document.getElementById('lfo2depth'); if(lfo2depthEl) lfo2depthEl.addEventListener('input', e=>{ if(lfo2Gain) lfo2Gain.gain.value = +e.target.value; });

    const crossLowMidEl = document.getElementById('crossLowMid'); if(crossLowMidEl) crossLowMidEl.addEventListener('input', e=>{ lowCrossover.value = +e.target.value; rebuildCrossovers(); });
    const crossMidHighEl = document.getElementById('crossMidHigh'); if(crossMidHighEl) crossMidHighEl.addEventListener('input', e=>{ highCrossover.value = +e.target.value; rebuildCrossovers(); });

    const lowGainEl = document.getElementById('lowGain'); if(lowGainEl) lowGainEl.addEventListener('input', e=>{ lowGainNode.gain.value = dbToGain(+e.target.value); });
    const midGainEl = document.getElementById('midGain'); if(midGainEl) midGainEl.addEventListener('input', e=>{ midGainNode.gain.value = dbToGain(+e.target.value); });
    const highGainEl = document.getElementById('highGain'); if(highGainEl) highGainEl.addEventListener('input', e=>{ highGainNode.gain.value = dbToGain(+e.target.value); });

    const lowDriveEl = document.getElementById('lowDrive'); if(lowDriveEl) lowDriveEl.addEventListener('input', e=>{ lowDriveNode.disconnect(); const d = makeDrive(+e.target.value); lowGainNode.connect(d); d.connect(masterGain); });
    const midDriveEl = document.getElementById('midDrive'); if(midDriveEl) midDriveEl.addEventListener('input', e=>{ midDriveNode.disconnect(); const d = makeDrive(+e.target.value); midGainNode.connect(d); d.connect(masterGain); });
    const highDriveEl = document.getElementById('highDrive'); if(highDriveEl) highDriveEl.addEventListener('input', e=>{ highDriveNode.disconnect(); const d = makeDrive(+e.target.value); highGainNode.connect(d); d.connect(masterGain); });

    const masterGainEl = document.getElementById('masterGain'); if(masterGainEl) masterGainEl.addEventListener('input', e=>{ masterGain.gain.value = dbToGain(+e.target.value); });
    const limiterEl = document.getElementById('limiterThreshold'); if(limiterEl) limiterEl.addEventListener('input', e=>{ masterCompressor.threshold.value = +e.target.value; });

    // Simple connections for modulation matrix checkboxes
    // We'll connect lfo1 to filter frequency and lfo2 to morph crossfade gains when enabled
    const lfo1ToFilterCheckbox = document.getElementById('lfo1ToFilter');
    const lfo2ToMorphCheckbox = document.getElementById('lfo2ToMorph');
    const lfo1ToDetuneCheckbox = document.getElementById('lfo1ToDetune');

    // Since filter frequency is not globally a single AudioParam per voice, we'll create a global filterGain node effected by lfo for demonstration
    // Instead, we will apply LFOs at playback time per voice in the voice class by reading checkbox states and modulating.

    // Keyboard UI: create 2 octaves (C3-B4)
    const keyNotes = ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
    const startOct = 3; const octaves = 2; const keyboard = document.getElementById('keyboard');
    const keyElements = [];
    for(let o=0;o<octaves;o++){
      for(let i=0;i<12;i++){
        const noteName = keyNotes[i]+(startOct+o);
        const midi = noteToMidi(noteName);
        const isBlack = keyNotes[i].includes('#');
        const el = document.createElement('div'); el.className = 'key'+(isBlack? ' black':''); el.dataset.midi = midi; el.innerText = keyNotes[i].replace('#','♯')+ (startOct+o);
        el.addEventListener('mousedown', ()=>noteOn(midi, 100));
        el.addEventListener('touchstart', (ev)=>{ ev.preventDefault(); noteOn(midi, 100); });
        el.addEventListener('mouseup', ()=>noteOff(midi));
        el.addEventListener('mouseleave', ()=>noteOff(midi));
        el.addEventListener('touchend', ()=>noteOff(midi));
        keyboard.appendChild(el); keyElements.push(el);
      }
    }

    // Computer keyboard mapping
    const keyMap = {
      'z':60,'s':61,'x':62,'d':63,'c':64,'v':65,'g':66,'b':67,'h':68,'n':69,'j':70,'m':71,
      ',':72,'l':73,'.':74,';':75,'/':76
    };
    const pressedKeys = new Set();
    window.addEventListener('keydown', e=>{
      const k = e.key.toLowerCase(); if(keyMap[k] && !pressedKeys.has(k)){ pressedKeys.add(k); noteOn(keyMap[k], 100); highlightKey(keyMap[k], true); }
    });
    window.addEventListener('keyup', e=>{ const k = e.key.toLowerCase(); if(keyMap[k]){ pressedKeys.delete(k); noteOff(keyMap[k]); highlightKey(keyMap[k], false); }});

    function highlightKey(midi, on){ const el = [...keyElements].find(k=>+k.dataset.midi===midi); if(el){ el.classList.toggle('pressed', on); }}

    // MIDI input support (optional)
    if(navigator.requestMIDIAccess){
      try{ const midiAccess = await navigator.requestMIDIAccess(); midiAccess.inputs.forEach(input=>{ input.onmidimessage = handleMIDIInput; }); }catch(e){console.log('MIDI access not granted');}
    }
    function handleMIDIInput(msg){ const [status, data1, data2] = msg.data; const cmd = status & 0xf0; if(cmd===0x90 && data2>0) noteOn(data1, data2); else if((cmd===0x80) || (cmd===0x90 && data2===0)) noteOff(data1); }

    function noteOn(midi, velocity){
      // ensure audio context resumed
      if(ctx.state === 'suspended'){ ctx.resume(); }
      if(state.activeNotes.has(midi)) return;
      // build per-voice params from global params
      const voiceParams = Object.assign({}, params, {
        filterMode: params.filterMode, filterFreq: params.filterFreq, filterQ: params.filterQ,
        subLevel: params.subLevel, noiseLevel: params.noiseLevel,
        amp: params.amp, filt: params.filt, filtEnvAmount: params.filtEnvAmount,
        layers: [params.layers[0], params.layers[1]]
      });
      const v = new Voice(midi, velocity, ctx, mixBus, voiceParams);
      state.activeNotes.set(midi, v);
      // visual
      highlightKey(midi, true);
    }

    function noteOff(midi){ if(!state.activeNotes.has(midi)) return; const v = state.activeNotes.get(midi); v.noteOff(); state.activeNotes.delete(midi); highlightKey(midi,false); }

    // Save/load presets
    const saveBtn = document.getElementById('savePreset'); const loadBtn = document.getElementById('loadPreset'); const exportBtn = document.getElementById('exportPreset'); const importInput = document.getElementById('importPreset');
    if(saveBtn) saveBtn.addEventListener('click', ()=>{
      const name = prompt('Preset name','Init'); if(!name) return;
      const preset = serializePreset();
      localStorage.setItem('preset_'+name, JSON.stringify(preset));
      alert('Saved preset "'+name+'" locally');
    });
    if(loadBtn) loadBtn.addEventListener('click', ()=>{
      const keys = Object.keys(localStorage).filter(k=>k.startsWith('preset_')); if(keys.length===0){ alert('No presets saved'); return; }
      const name = prompt('Load preset (enter exact name)\nAvailable:\n'+keys.map(k=>k.replace('preset_','')).join('\n')); if(!name) return; const key='preset_'+name; if(!localStorage.getItem(key)){ alert('Not found'); return; }
      const preset = JSON.parse(localStorage.getItem(key)); loadPreset(preset); alert('Loaded '+name);
    });
    if(exportBtn) exportBtn.addEventListener('click', ()=>{ const preset = serializePreset(); const blob = new Blob([JSON.stringify(preset,null,2)],{type:'application/json'}); const url = URL.createObjectURL(blob); const a = document.createElement('a'); a.href = url; a.download = 'wavetable-synth-preset.json'; a.click(); URL.revokeObjectURL(url); });
    if(importInput) importInput.addEventListener('change', e=>{ const f = e.target.files[0]; if(!f) return; const reader = new FileReader(); reader.onload = ()=>{ const p = JSON.parse(reader.result); loadPreset(p); alert('Imported preset'); }; reader.readAsText(f); });

    function serializePreset(){
      return {
        oscA: params.oscA, oscB: params.oscB,
        layers: [{type:params.layers[0] && params.layers[0].type, morph: params.layers[0].morph},{type:params.layers[1] && params.layers[1].type, morph: params.layers[1].morph}],
        subLevel: params.subLevel, noiseLevel: params.noiseLevel, stereoWidth: params.stereoWidth,
        filterMode: params.filterMode, filterFreq: params.filterFreq, filterQ: params.filterQ,
        amp: params.amp, filt: params.filt,
        crossLowMid: lowCrossover.value, crossMidHigh: highCrossover.value,
      };
    }
    function loadPreset(p){ if(!p) return; if(p.oscA) params.oscA = p.oscA; if(p.oscB) params.oscB = p.oscB; if(p.subLevel!==undefined) params.subLevel=p.subLevel; if(p.noiseLevel!==undefined) params.noiseLevel=p.noiseLevel; if(p.filterMode) params.filterMode=p.filterMode; if(p.filterFreq) params.filterFreq=p.filterFreq; if(p.filterQ) params.filterQ=p.filterQ; if(p.amp) params.amp=p.amp; if(p.filt) params.filt=p.filt; if(p.crossLowMid) document.getElementById('crossLowMid').value = p.crossLowMid; if(p.crossMidHigh) document.getElementById('crossMidHigh').value = p.crossMidHigh; updateLayer(0); updateLayer(1); rebuildCrossovers(); }

    // Visualization (oscilloscope basic)
    const analyser = ctx.createAnalyser(); analyser.fftSize = 2048; mixBus.connect(analyser);
    const canvas = document.getElementById('viz'); const cctx = canvas ? canvas.getContext('2d') : null;
    function draw(){ requestAnimationFrame(draw); if(!cctx) return; const data = new Uint8Array(analyser.fftSize); analyser.getByteTimeDomainData(data); cctx.fillStyle = '#071017'; cctx.fillRect(0,0,canvas.width,canvas.height); cctx.lineWidth=2; cctx.strokeStyle='#6ee7b7'; cctx.beginPath(); const slice = canvas.width / data.length; let x=0; for(let i=0;i<data.length;i++){ const v = data[i]/128.0; const y = (v*canvas.height)/2; if(i===0) cctx.moveTo(x,y); else cctx.lineTo(x,y); x+=slice; } cctx.stroke(); }
    draw();

    // misc helpers
    function midiToFreq(m){ return 440 * Math.pow(2,(m-69)/12); }
    function noteToMidi(name){ // supports C4 etc
      const match = name.match(/^([A-G]#?)(-?\d)$/);
      if(!match) return 60;
      const pitch=match[1]; const octave=+match[2]; const order = {'C':0,'C#':1,'D':2,'D#':3,'E':4,'F':5,'F#':6,'G':7,'G#':8,'A':9,'A#':10,'B':11};
      return 12*(octave+1) + order[pitch]; }

    // basic unit test generator for demo tones (procedural test tones)
    window.generateTestTone = function(freq=440,dur=1){ const o = ctx.createOscillator(); const g = ctx.createGain(); o.frequency.value = freq; g.gain.value = 0.5; o.connect(g); g.connect(mixBus); o.start(); g.gain.setValueAtTime(0.5, ctx.currentTime); g.gain.linearRampToValueAtTime(0.0, ctx.currentTime+dur); setTimeout(()=>{try{o.stop();}catch(e){}}, dur*1000+200); };

    // initial UI states
    const fModeEl = document.getElementById('filterMode'); if(fModeEl) fModeEl.value = params.filterMode;
    const fFreqEl2 = document.getElementById('filterFreq'); if(fFreqEl2) fFreqEl2.value = params.filterFreq;
    const fQEl2 = document.getElementById('filterQ'); if(fQEl2) fQEl2.value = params.filterQ;

    // Expose basic debug controls
    window.__synth = {ctx, params, generateTestTone, state};

    console.log('Wavetable Dual Synth (Web) ready');
  }catch(err){
    console.error('Synth init error', err);
  }

})();