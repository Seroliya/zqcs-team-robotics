const {chromium}=require('C:/Users/15300/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/playwright');
(async()=>{
  const browser=await chromium.launch({headless:true,executablePath:'C:/Program Files/Google/Chrome/Application/chrome.exe'});
  const page=await browser.newPage();
  const errors=[];page.on('pageerror',e=>errors.push(e.message));
  for(const width of [736,320]){
    await page.setViewportSize({width,height:760});
    await page.goto('file:///C:/Users/15300/Documents/codex_repos/CRTC/tmp/sources/chassis-preview.html');
    const frame=page.frames().find(f=>f.parentFrame());
    await frame.locator('#crtc-marks text').first().waitFor();
    await frame.locator('#crtc-width').evaluate(el=>{el.value='135';el.dispatchEvent(new Event('input',{bubbles:true}));});
    await frame.locator('#crtc-count').evaluate(el=>{el.value='9';el.dispatchEvent(new Event('input',{bubbles:true}));});
    const metrics=await frame.evaluate(()=>({width:document.documentElement.clientWidth,scroll:document.documentElement.scrollWidth,blocks:document.querySelectorAll('.block').length,state:document.getElementById('crtc-state').textContent,svgOverflow:[...document.querySelectorAll('.crtc-plan text')].filter(el=>{const b=el.getBoundingClientRect(),s=document.querySelector('.crtc-plan').getBoundingClientRect();return b.left<s.left-1||b.right>s.right+1;}).map(el=>el.textContent)}));
    if(metrics.scroll>metrics.width||metrics.blocks!==9||metrics.svgOverflow.length)throw new Error(JSON.stringify(metrics));
    await page.screenshot({path:`C:/Users/15300/Documents/codex_repos/CRTC/tmp/sources/preview-${width}.png`,fullPage:true});
    await frame.locator('#crtc-width').evaluate(el=>{el.value='190';el.dispatchEvent(new Event('input',{bubbles:true}));});
    const changed=await frame.locator('#crtc-state').textContent();
    if(!changed.includes('20.0'))throw new Error('Width interaction did not update');
    console.log(JSON.stringify({viewport:width,...metrics,changed}));
  }
  if(errors.length)throw new Error(errors.join('; '));
  await browser.close();
})().catch(e=>{console.error(e);process.exit(1);});
