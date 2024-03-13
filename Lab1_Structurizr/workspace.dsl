workspace {

    name "Социальная сеть"
    description "Лабораторная работа №1:  Проектирование программной системы"
    
    !identifiers hierarchical
    
    !docs documentation
    !adrs decisions

    model {
        properties { 
            structurizr.groupSeparator "/"
        }
        
        user = person "Пользователь" { 
            description "Пользователь, использующий социальную сеть"
        }
        socialNetwork = softwareSystem "Социальная сеть" {
            description "Приложение социальной сети"
            
            user_app = container "User app" {
                description "Приложение, с которым взаимодействует пользователь"
            }
            
            user_service = container "User serviсe" {
                description "Сервис управления данными пользователя"
            }
            
            message_service = container "Message service" {
                description "Сервис управления данными сообщений"
            }
            
            post_service = container "Post service" {
                description "Сервис управления данними постов"
            }
            
            group "Слой данных" {
                user_database = container "User Database" {
                    description "База данных с пользователями"
                    technology "PostgreSQL"
                    tags "database"
                }

                user_cache = container "User Cache" {
                    description "Кеш пользовательских данных для ускорения поиска по ключевым полям"
                    technology "Redis"
                    tags "database"
                }

                socialNetwork_database = container "socialNetwork Database" {
                    description "База данных для хранения информации со стены и чатов пользователя"
                    technology "MongoDB"
                    tags "database"
                    
                    message_collection = component "Коллекция сообщений пользователей"{
                        tags "database"
                    }
                    post_collection = component "Коллекция постов пользователей"{
                        tags "database"
                    }
                }
            }
            
            
            user -> user_app "Регистрация пользователя/ написание сообщения/ создание поста" "REST HTTP:8080"
            user_app -> user_service "Регистрация нового пользователя"
            user_app -> message_service "Написание сообщения"
            user_app -> post_service "Создание/удаление поста"
            
            user_service -> user_cache "Получение/обновление данных о пользователях"
            user_service -> user_database "Получение/обновление данных о пользователях"
            
            message_service -> socialNetwork_database "Получение/обновление данных о сообщениях"
            post_service -> socialNetwork_database "Получение/обновление данных о постах"
            
        }
        user -> socialNetwork "Использование приложения: регистрация, создание постов, переписки в чате"
        
        production = deploymentEnvironment "Production" {
        
            deploymentNode "User Server" {
    
                client_service = deploymentNode "User service" {
                    containerInstance socialNetwork.user_service
                }
                
                message_service = deploymentNode "Message service" {
                    containerInstance socialNetwork.message_service
                }
    
                post_service = deploymentNode "Post service" {
                    containerInstance socialNetwork.post_service
                }
    
                user_app = deploymentNode "User app" {
                    containerInstance socialNetwork.user_app
                    properties {
                        "cpu" "4"
                        "ram" "256Gb"
                        "hdd" "1Tb"
                    }
                }
    
                client_service -> user_app
                message_service -> user_app
                post_service -> user_app
            }
        

            deploymentNode "databases" {
     
                deploymentNode "Database Server 1" {
                    containerInstance socialNetwork.user_database
                    tags "database"
                }

                deploymentNode "Database Server 2" {
                    containerInstance socialNetwork.socialNetwork_database
                    instances 3
                    tags "database"
                }

                deploymentNode "Cache Server" {
                    containerInstance socialNetwork.user_cache
                    tags "database"
                }
            }
            
        }
    
    }

    views {
        systemContext socialNetwork "socialNetwork" {
            include *
            autoLayout
        }
        
        container socialNetwork {
            autoLayout
            include *
        }
        
        deployment socialNetwork production {
            autoLayout
            include *
        }
        
        dynamic socialNetwork "UC01" "Создание нового пользователя" {
            autoLayout
            user -> socialNetwork.user_app "Создать нового пользователя (POST /user)"
            socialNetwork.user_app -> socialNetwork.user_service "Перенаправить запрос"
            socialNetwork.user_service -> socialNetwork.user_database "Сохранить данные о пользователе"
            socialNetwork.user_service -> socialNetwork.user_cache "Кэшировать данные о пользователе" 
        }
        
        dynamic socialNetwork "UC02" "Поиск пользователя по логину" {
            autoLayout
            user -> socialNetwork.user_app "Поиск пользователя по логину (GET /user)"
            socialNetwork.user_app -> socialNetwork.user_service "Перенаправить запрос"
            socialNetwork.user_service -> socialNetwork.user_database "Получить данные о пользователе" 
            socialNetwork.user_service -> socialNetwork.user_cache "Получить данные о пользователе из кэша" 
        }
        
        dynamic socialNetwork "UC03" "Поиск пользователя по маске имени и фамилии" {
            autoLayout
            user -> socialNetwork.user_app "Поиск пользователя по маске имени и фамилии (GET /user)"
            socialNetwork.user_app -> socialNetwork.user_service "Перенаправить запрос"
            socialNetwork.user_service -> socialNetwork.user_database "Получить данные о пользователе" 
        }
        
        dynamic socialNetwork "UC04" "Добавление записи на стену" {
            autoLayout
            user -> socialNetwork.user_app "Создать пост (POST /user/post)"
            socialNetwork.user_app -> socialNetwork.post_service "Перенаправить запрос"
            socialNetwork.post_service -> socialNetwork.socialNetwork_database "Сохранить данные о посте" 
        }
        
        dynamic socialNetwork "UC05" "Загрузка стены пользователя" {
            autoLayout
            user -> socialNetwork.user_app "Запрос на загрузку стены (GET /user/post)"
            socialNetwork.user_app -> socialNetwork.post_service "Перенаправить запрос"
            socialNetwork.post_service -> socialNetwork.socialNetwork_database "Получть данные о постах польхователя" 
        }
        
        dynamic socialNetwork "UC06" "Отправка сообщения пользователю" {
            autoLayout
            user -> socialNetwork.user_app "Отправить сообщение пользователю (POST /user/message)"
            socialNetwork.user_app -> socialNetwork.message_service "Перенаправить запрос"
            socialNetwork.message_service -> socialNetwork.socialNetwork_database "Сохранить данные о сообщении" 
        }
        
        dynamic socialNetwork "UC07" "Получение списка сообщений для пользователя" {
            autoLayout
            user -> socialNetwork.user_app "Запрос на загрузку сообщений (GET /user/message)"
            socialNetwork.user_app -> socialNetwork.message_service "Перенаправить запрос"
            socialNetwork.message_service -> socialNetwork.socialNetwork_database "Получить данные о сообщениях пользователя" 
        }

        styles {
            element "Software System" {
                background #1168bd
                color #ffffff
            }
            element "Person" {
                shape person
                background #08427b
                color #ffffff
            }
        }
    }
    
}
