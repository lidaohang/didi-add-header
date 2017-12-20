### 目标
动态添加header头

### 方案

#### 介绍

根据配置动态从header头、url参数、变量获取值，插入到header头didi-header-hint-content中


### 使用说明

#### 指令说明


```

Syntax: didi_add_header on|off;
Default:    —
Context:    main, http, server, location, if in location


Syntax: didi_add_header_type header|param|variable;
Default:    —
Context:    main, http, server, location, if in location

#param:         获取url中的参数的值
#header:        获取header中的字段的值
#variable:      获取变量名的值


Syntax: didi_add_header_name name rename(可选);
Default:    —
Context:    main, http, server, location, if in location


```



#### Nginx配置


```
    upstream didi.hnq_80 {
        server 10.94.106.240:8011;
    }

    upstream didi.hna_80 {
        server 10.94.106.240:8012;
    }

    didi_add_header  on; #开关
    didi_add_header_type variable; #变量类型

    didi_smart_routing on;  #是否启用开关
    didi_smart_routing_names b2b_test; #路由名称列表，支持多集群

    server {
        listen       80;
        server_name  localhost;

        set $cityid "123";

        location / {

            set $Cityid "456";
            set_by_lua $Cityid 'return 20001';

            didi_add_header_name Cityid;  #名称

            #路由 {main_upstream_name}.{$lidc}_{port}
            didi_smart_routing_upstream main_upstream_name=didi default_lidc=hna  default_port=80;

            proxy_pass http://$upstream_name;
        
        }
    
    }
```



#### 测试方式

 - 请求到nginx如下：
```
curl 'http://127.0.0.1/mock/getinfo' -H 'Cityid: "1001"'  'didi-header-hint-content:{"k1": "v1"}' 
```

 - 转发到后端服务器

```
curl 'http://127.0.0.1/mock/getinfo' -H 'didi-header-hint-content:{"k1":"v1","Cityid": "1001"}' 
```
