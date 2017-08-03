Feature: Project rename
  As someone collecting tasks
  I can rename a project
  In order to refine my tasks organization

  Scenario: Renamed projects appear in the list
    Given I display the available pages
    When I rename the page named "Backlog" under "Projects / TestData » Calendar1 » Calendar2" to "Party"
    And I list the items
    Then the list is:
       | display                                                | icon              |
       | Inbox                                                  | mail-folder-inbox |
       | Workday                                                | go-jump-today     |
       | Projects                                               | folder            |
       | Projects / TestData » Calendar1                          | folder            |
       | Projects / TestData » Calendar1 / Prepare talk about TDD | view-pim-tasks    |
       | Projects / TestData » Calendar1 / Read List              | view-pim-tasks    |
       | Projects / TestData » Calendar1 » Calendar2                | folder            |
       | Projects / TestData » Calendar1 » Calendar2 / Party        | view-pim-tasks    |
       | Contexts                                               | folder            |
       | Contexts / Errands                                     | view-pim-notes    |
       | Contexts / Online                                      | view-pim-notes    |
