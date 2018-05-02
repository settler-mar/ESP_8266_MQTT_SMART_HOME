var rule_list = {};
var spa_page = (function() {
  var base_url='/';
    var option ={
      all_section: false,
      def:false,
      wrapper:false,
      nav:false,
      nav_url_last:false,
      costume_function:{},
      fn:{},
    }


    function onhashchange() {
      var hash = location.hash || option.def;
      if(hash.length<2)hash=option.def;

      var re = /#([-0-9A-Za-z]+)(\:(.+))?/;
      var match = re.exec(hash);
      hash = match[1];
      var param = match[3];
      this.fn.show(hash,param);
    }

    function show(pageName,param) {
      this.all_section=document.querySelectorAll('section');
      this.all_section.forEach(function(el){
        el.classList.remove("show");
        content=el.getElementsByClassName('content')
        content[0].innerHTML="";
      })

      if(!(this_page=document.getElementById(pageName))){
        this_page='<section id="'+pageName+'"><h2><span class="bg"></span><span class="text">'+pageName+'</span></h2><div class="content"></div></section>';
        this.wrapper.innerHTML+=this_page
        this_page=document.getElementById(pageName);

        this.all_section=document.querySelectorAll('section');
      }
      getPost('pages/'+pageName+'.json','',function(data){
        data=JSON.parse(data)
        if(data.title){
          var this_page_title=this_page.querySelectorAll('h2 .text')[0]
          this_page_title.innerHTML=data.title
        }

        this_page.classList.add("show");

        if(data.nav && data.nav!=undefined){
          spa_page.set_menu(data.nav)
        }

        if(data.url){
          url=data.url;
          if(param && param!=undefined)url+='?'+param;
          getPost(url,'',function(json){
            json=JSON.parse(json)
            render_blk.bind(this)(pageName,data,json,url)
          }.bind(this))
        }else{
          url='/'+pageName;
          render_blk.bind(this)(pageName,data,{},url)
        }
      }.bind(this),"GET")
    }

    function render_blk(pageName,data,json,url){
      if(!data.url)data.url='/'+pageName;
      if(data.type=="table"){
        generateTable(pageName,data.data,json)
        return;
      }
      if(data.type=="form"){
        generateForm(pageName,data.data,json,data)
        return;
      }
      if(this.costume_function[data.type])this.costume_function[data.type](pageName,data)
    }
    /*function app(pageName,param) {
      this.fn.show(pageName,param);
    }*/

    function set_menu(data){
      if(typeof(data)=="object"){
        html='';
        var nav=document.querySelectorAll(this.nav)[0];
        for(i=0;i<data.length;i++){
          html+="<a href='"+data[i].url+"'>"+data[i].title+"</a>"
        }
        nav.innerHTML=html
        return;
      }

      if(data && data!=undefined && this.nav_url_last!=data){
        this.nav_url_last=data
        getPost(data,'',function(data){
          data=JSON.parse(data)
          this.fn.set_menu(data)
        }.bind(this))
      }
    }

    function init(params){
      base_url='/';
      if(params){
        if(params.nav){
          this.nav=params.nav.parant;
          this.fn.set_menu(params.nav.url)
        }
        if(params.costume_function){
          this.costume_function=params.costume_function;
        }
        if(!params.costume_function ||
          !params.costume_function.base_url ||
          typeof(params.costume_function.base_url)=="undefined")
        {
          base_url="/";
        }else{
          base_url=params.costume_function.base_url;
          if(base_url[base_url.length-1]!='/'){
            base_url+='/';
          }
        }
      }

      this.all_section=document.querySelectorAll('section');
      this.def=document.querySelectorAll('section[default]')[0];
      this.def="#"+option.def.id;
      this.wrapper = document.querySelectorAll('.wrapper')[0];

      window.addEventListener("hashchange", this.fn.onhashchange);
      window.setTimeout( this.fn.onhashchange);
    }

    option.fn={
      init:init.bind(option),
      show:show.bind(option),
      set_menu:set_menu.bind(option),
      //app:app.bind(option),
      onhashchange:onhashchange.bind(option),
    };
    return option.fn;
  }())

  function getXmlHttp() {
    var xmlhttp;
    try {
      xmlhttp = new ActiveXObject("Msxml2.XMLHTTP");
    } catch (e) {
    try {
      xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    } catch (E) {
      xmlhttp = false;
    }
    }
    if (!xmlhttp && typeof XMLHttpRequest!='undefined') {
      xmlhttp = new XMLHttpRequest();
    }
    return xmlhttp;
  }

  function getPost(url,data,callback,type) {
    var xmlhttp = getXmlHttp(); // Создаём объект XMLHTTP
    if(url[0]=='/'){
      url=url.substring(1);
    }
    if(!base_url || typeof(base_url)=="undefined"){
      var base_url="/";
    }
    xmlhttp.open(type||'POST', base_url+url, true); // Открываем асинхронное соединение
    xmlhttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); // Отправляем кодировку
    if(typeof(data)!="string"){
      data=arrayToQueryString(data);
    }
    xmlhttp.send(data); // Отправляем POST-запрос
    xmlhttp.onreadystatechange = function() { // Ждём ответа от сервера
      if (xmlhttp.readyState == 4) { // Ответ пришёл
        if(xmlhttp.status == 200) { // Сервер вернул код 200 (что хорошо)
          callback(xmlhttp.responseText); // Выводим ответ сервера
        }
      }
    };
  }

/*function loadDoc() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
     document.getElementById("demo").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "ajax_info.txt", true);
  xhttp.send();
}*/

function arrayToQueryString(array_in){
  var out = new Array();
  for(var key in array_in){
    if(typeof(array_in[key])=="object"){
      for(var k in array_in[key]){
        out.push(key + '[]=' + encodeURIComponent(array_in[key][k]));
      }
    }else{
      out.push(key + '=' + encodeURIComponent(array_in[key]));
    }
  }
  return out.join('&');
}

function generateTable(pageName,data,json,echo){
  if(echo!==false)echo=true;
  var out;

  var input_blk=document.querySelectorAll('#'+pageName+" .content")[0];
  var out="<tr>";
  for(i=0;i<data.length;i++){
      out+="<th>"+data[i].title+"</th>"
  }
  out+="</tr>"


  for(j=0;j<json.length;j++){
    out+="<tr>"
    for(i=0;i<data.length;i++){
        out+="<td class='type_"+data[i].type+"'>";
        out+=genControl(data[i],json[j]);
        out+="</td>";
    }
    out+="</tr>"
  }
  out='<table>'+out+'</table>'
  if(echo){
    input_blk.innerHTML=out
  }else{
    return out
  }
}

function generateForm(pageName,data,json,source){
  var out;
  rule_list = {}
  url=source.url
  var input_blk=document.querySelectorAll('#'+pageName+" .content")[0];
  var out="<form method='post' action="+url+">";
  for(i=0;i<data.length;i++){

    if(data[i].type=="table"){
      out+=generateTable(pageName,data[i].data,json[data[i].name],false)
      continue;
    }

    if(data[i].type=="hidden"){
      out+="<input name="+data[i].name+" type=hidden value='"+json[data[i].name]+"'>"
      continue;
    }

    if(data[i].type!="radio" && data[i].type!="checkbox"){
      baze_tag="label"
    }else{
      baze_tag="div"
    }

    out+="<"+baze_tag+" class=label"
    if(data[i].title)out+=" text='"+data[i].title+"'"
    out+=">"

    if(data[i].prefix)out+=data[i].prefix;

    out+=genControl(data[i],json);

    if(data[i].rule){
      rule_list[data[i].name]=data[i].rule;
    }

    if(data[i].sufix)out+=data[i].sufix;
    out+="</"+baze_tag+">";
  }
  submitName=source.submitName||"Save";
  out+="<input type='submit' value='"+submitName+"'>"
  out+="</form>";
  input_blk.innerHTML=out

  test_rule();
  var form=input_blk.querySelectorAll('form')[0]
  form.onsubmit = function(e) {
    e.preventDefault()
    form=this;
    post=serlise_form(form);
    form.classList.add("loading");
    getPost(url,post,function(json){
      form.classList.remove("loading");
      json=JSON.parse(json)
      if(typeof(json_alert)!="undefined"){
        json_alert.analis(json)
      }else{
        if(json.msg){
          showAlert(json.msg);
        }
      }
      generateForm(pageName,data,json,source)
    })
    return false;
  }
}

function genControl(data,json){
  var out='';
  if(data.type=="text"){
    out+=json[data.name]
  }
  if(data.type=="input"){
    out+="<input name="+data.name+" type=text value='"+(json[data.name]||"")+"'>"
  }
  if(data.type=="multiinput"){
    value=json[data.name]||"";
    if(typeof(value)!="object"){
      value=[value];
    }
    if(value.length==0 || value[value.length-1].length!=0){
      value.push("")
    }
    for (j=0;j<value.length;j++){
      dop_attr=''
      if(data.maxLine){
        dop_attr=" data-max-line="+data.maxLine+" "
      }
      out+="<input name="+data.name+dop_attr+" class='multiinput' type=text value='"+value[j]+"' onchange='test_multiinput(this)' onkeyup='test_multiinput(this,true)'>"
    }
  }
  if(data.type=="password"){
    out+="<input name="+data.name+" type=password >"
  }
  if (data.type=="control"){
    out+="<div class=remove onclick='remove_line'>"+(data.text||"Remove")+"</div>"
  }
  if(data.type=="radio" || data.type=="checkbox"){
    dop_to_span='';

    if(data.width){
      dop_to_span+="style='width:"+data.width+"' "
    }
    if(data.state){
      dop_to_span+="on='"+data.state.on+"' "
      dop_to_span+="off='"+data.state.off+"' "
    }
    val=json[data.name];
    for (var j in  data.value) {
      j=parseInt(j);
      if(typeof(val)=="object"){
        ch=val.indexOf(j)>=0?" checked ":"";
      }else{
        ch=val==j?" checked ":"";
      }
      out+="<label class=radio>"
      out+="<input name="+data.name+" type="+data.type+" value='"+j+"'"+ch+" onchange='test_rule()'>"
      out+="<span "+dop_to_span+">"+data.value[j]+"</span>"
      out+="</label>"
    }
  }
  if(data.type=="href"){
    href=json[data.name]
    txt=json[data.description]||href

    if(href)out+='<a href="'+href+'">'
    out+=txt
    if(href)out+='</a>'
  }
  return out;
}

function showAlert(txt){
  alert='<div class="alert">'+txt+'</div>'
  document.querySelectorAll('.wrapper')[0].innerHTML+=alert;
  setTimeout(function(){
    alert=document.querySelectorAll('.alert')[0];
    alert.remove()
  },2000)
}

function serlise_form(form){
  out={}

  els=form.querySelectorAll('[type=text],[type=hidden],[type=password],[type=radio]:checked,[type=checkbox]:checked')
  for(i=0;i<els.length;i++){
    if(els[i].value.length>0){
      if(!out[els[i].name]){
        out[els[i].name]=els[i].value
      }else{
        if(typeof(out[els[i].name])=="object"){
          out[els[i].name].push(els[i].value);
        }else{
          out[els[i].name]=[out[els[i].name],els[i].value]
        }
      }
    }

  }

  return arrayToQueryString(out);
}

function test_rule(){
  var t_val={}
  var t_val={}
  for (var j in  rule_list) {
    sel=true;
    for (var i in  rule_list[j]) {
      if(t_val[i]==undefined){
        t_val[i]=document.querySelectorAll('[name='+i+']:checked')
        t_val[i]=t_val[i].length>0?t_val[i][0].value:"";
      }
      sel=sel && (t_val[i]==rule_list[j][i]);
    }
    el=document.querySelectorAll('[name='+j+']')[0].parentNode;
    if(!el.hasClass('label'))el=el.parentNode
    if(sel){
      el.classList.remove("hiden");
    }else{
      el.classList.add("hiden");
    }
  }
}

function test_multiinput(el,not_del){
  mt_arr=el.parentNode.querySelectorAll(':not(.anim_hide)');
  if(!not_del && el!=mt_arr[mt_arr.length-1] && el.value.length<1){
    animateAndRemove(el,'anim_hide')
    return;
  }

  if(el==mt_arr[mt_arr.length-1] && el.value.length>0){

    if(el.dataset.maxLine && mt_arr.length>=el.dataset.maxLine)return

    clone_el=mt_arr[0].cloneNode()
    clone_el.value=''
    clone_el.classList.remove('anim_show');
    el.after(clone_el)
    clone_el.classList.add('anim_show');
    return;
  }

  if(el!=mt_arr[mt_arr.length-1] && el.value.length==0 && mt_arr[mt_arr.length-1].value.length==0){
    animateAndRemove(mt_arr[mt_arr.length-1],'anim_hide')
  }
}

function addClassWithCalback(element,className,callback){
  var ev_name = ["animationend", "animationend", "webkitAnimationEnd", "oanimationend", "MSAnimationEnd"];
  element.classList.remove(className);
  for(var i=0;i<ev_name.length;i++){
    element.addEventListener(ev_name[i], callback, false);
  }
  element.classList.add(className);
}

function animateAndRemove(element,className){
  var remove_el=function(){
    this.remove()
  }
  addClassWithCalback(element,className,remove_el)
}

HTMLElement.prototype.hasClass = function(cls) {
    var i;
    var classes = this.className.split(" ");
    for(i = 0; i < classes.length; i++) {
        if(classes[i] == cls) {
            return true;
        }
    }
    return false;
};
