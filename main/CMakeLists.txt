set(SRC 
       "epd.c"
       "gui.c"
       "ntp.c"
       "dht.c"
       "rss.c"
       "main.c"
       "wifi.c"
       "scrn.c"
       "stock.c"
   )

idf_component_register(SRCS ${SRC}
                       EMBED_TXTFILES rss_cert.pem stock_cert.pem
                       INCLUDE_DIRS ".")
